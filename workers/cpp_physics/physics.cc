#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>

#include <improbable/view.h>
#include <improbable/worker.h>

#include <improbable/standard_library.h>
#include <client_data.h>

using ComponentRegistry =
    worker::Components<improbable::Position, improbable::Metadata, improbable::EntityAcl,
                       sample::Login, sample::ClientData>;

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

  view.OnDisconnect([&](const worker::DisconnectOp& op) {
    std::cerr << "[disconnect] " << op.Reason << std::endl;
    is_connected = false;
  });
  view.OnCommandRequest<sample::Login::Commands::TakeControl>(
      [&](const worker::CommandRequestOp<sample::Login::Commands::TakeControl>& op) {
        connection.SendLogMessage(worker::LogLevel::kInfo, "Physics",
                                  "Assigning ClientData component to " + op.CallerAttributeSet[1]);

        // Assign write ACL of the client data component to the worker ID received.
        improbable::EntityAcl::Update acl_update;
        auto current_write_acl =
            view.Entities[entityId].Get<improbable::EntityAcl>()->component_write_acl();
        current_write_acl[sample::ClientData::ComponentId] =
            improbable::WorkerRequirementSet({{{op.CallerAttributeSet[1]}}});
        acl_update.set_component_write_acl(current_write_acl);
        connection.SendComponentUpdate<improbable::EntityAcl>(entityId, acl_update);
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
    angle += 0.5;
    improbable::Position::Update position_update;
    position_update.set_coords({std::sin(angle) * 100.0, 0.0, std::cos(angle) * 100.0});
    connection.SendComponentUpdate<improbable::Position>(entityId, position_update);

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
