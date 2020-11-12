#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>

#include <improbable/view.h>
#include <improbable/worker.h>

#include <improbable/restricted/system_components.h>
#include <improbable/standard_library.h>
#include <client_data.h>

// We are using the PhysicsSimulationSet and ClientSimulationSet component sets for delegating
// authority. This includes all user-defined components at the moment. We also need some
// standard-library components which we don't currently delegate to anyone, so we include the
// WellKnownComponentSet and some components from the improbable::restricted namespace.
using ComponentRegistry =
    worker::Schema<sample::PhysicsSimulationSet, sample::ClientSimulationSet,
                   improbable::WellKnownComponentSet, improbable::restricted::Worker,
                   improbable::restricted::Partition>;

const int kErrorExitStatus = 1;
const std::uint32_t kOpListTimeoutMs = 100;

int main(int argc, char** argv) {
  auto print_usage = [&]() {
    std::cout << "Usage: ./Physics <hostname> <port> <worker_id>" << std::endl;
    std::cout << std::endl;
    std::cout << "Connects to SpatialOS" << std::endl;
    std::cout << "    <hostname>      - hostname of the receptionist to connect to." << std::endl;
    std::cout << "    <port>          - port to use if connecting through the receptionist."
              << std::endl;
    std::cout << "    <worker_id>     - name of the worker assigned by SpatialOS." << std::endl;
    std::cout << std::endl;
  };

  if (argc != 4) {
    print_usage();
    return kErrorExitStatus;
  }

  worker::EntityId entityId = 1;
  worker::EntityId physicsPartitionId = 2;
  worker::EntityId clientPartitionId = 3;

  worker::ConnectionParameters parameters;
  parameters.WorkerType = "physics";
  parameters.Network.ConnectionType = worker::NetworkConnectionType::kKcp;
  parameters.Network.Kcp.SecurityType = worker::NetworkSecurityType::kInsecure;
  parameters.Network.UseExternalIp = false;

  worker::LogsinkParameters logsink_params;
  logsink_params.Type = worker::LogsinkType::kStdout;
  auto log_filter = [](worker::LogCategory categories, worker::LogLevel level) -> bool {
    return level >= worker::LogLevel::kWarn ||
        (level >= worker::LogLevel::kInfo && categories & worker::LogCategory::kLogin);
  };
  logsink_params.FilterParameters.CustomFilter = log_filter;
  parameters.Logsinks.emplace_back(logsink_params);
  parameters.EnableLoggingAtStartup = true;

  worker::Connection connection =
      worker::Connection::ConnectAsync(ComponentRegistry{}, std::string{argv[1]}, atoi(argv[2]),
                                       argv[3], parameters)
          .Get();

  if (connection.GetConnectionStatusCode() != worker::ConnectionStatusCode::kSuccess) {
    std::cerr << "Failed to connect to the receptionist." << std::endl;
    std::cerr << "Reason: " << connection.GetConnectionStatusDetailString() << std::endl;
    return kErrorExitStatus;
  }
  std::cout << "Successfully connected using the Receptionist" << std::endl;

  worker::View view{ComponentRegistry{}};
  bool is_connected = true;

  using AssignPartitionCommand = improbable::restricted::Worker::Commands::AssignPartition;
  // In real code, we would probably want to retry here.
  view.OnCommandResponse<AssignPartitionCommand>(
      [&](const worker::CommandResponseOp<AssignPartitionCommand>& op) {
        if (op.StatusCode == worker::StatusCode::kSuccess) {
          connection.SendLogMessage(worker::LogLevel::kInfo, "Physics",
                                    "Successfully assigned partition.");
        } else {
          connection.SendLogMessage(worker::LogLevel::kError, "Physics",
                                    "Failed to assign partition: error code : " +
                                        std::to_string(static_cast<std::uint8_t>(op.StatusCode)) +
                                        " message: " + op.Message);
        }
      });

  connection.SendCommandRequest<improbable::restricted::Worker::Commands::AssignPartition>(
      connection.GetWorkerEntityId(), {physicsPartitionId},
      /* default timeout */ {});

  view.OnDisconnect([&](const worker::DisconnectOp& op) {
    std::cerr << "[disconnect] " << op.Reason << std::endl;
    is_connected = false;
  });
  view.OnCommandRequest<sample::Login::Commands::TakeControl>(
      [&](const worker::CommandRequestOp<sample::Login::Commands::TakeControl>& op) {
        connection.SendLogMessage(worker::LogLevel::kInfo, "Physics",
                                  "Assigning the client partition to worker with ID " +
                                      std::to_string(op.CallerWorkerEntityId));
        connection.SendCommandRequest<AssignPartitionCommand>(op.CallerWorkerEntityId,
                                                              {clientPartitionId},
                                                              /* default timeout */ {});
      });
  view.OnCommandResponse<sample::ClientData::Commands::TestCommand>(
      [&](const worker::CommandResponseOp<sample::ClientData::Commands::TestCommand>& op) {
        if (op.StatusCode == worker::StatusCode::kSuccess) {
          connection.SendLogMessage(
              worker::LogLevel::kInfo, "Physics",
              "Received command response: " + std::to_string(op.Response->sum()));
        }
      });

  int tick_count = 0;
  double angle = 0.0;
  while (is_connected) {
    view.Process(connection.GetOpList(kOpListTimeoutMs));

    // Update position of entity.
    if (view.GetAuthority<sample::PhysicsSimulationSet>(entityId) ==
        worker::Authority::kAuthoritative) {
      angle += 0.5;
      improbable::Position::Update position_update;
      position_update.set_coords({std::sin(angle) * 100.0, 0.0, std::cos(angle) * 100.0});
      connection.SendComponentUpdate<improbable::Position>(entityId, position_update);
    }

    // Sleep for some time.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Send a command every 2 ticks.
    if (tick_count++ % 2 == 0) {
      connection.SendCommandRequest<sample::ClientData::Commands::TestCommand>(entityId, {10, 1.5f},
                                                                               {});
    }
  }

  return kErrorExitStatus;
}
