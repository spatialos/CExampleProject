using System;
using System.Reflection;
using Improbable;
using Improbable.Collections;
using Improbable.Worker;
using Sample;

internal class Physics
{
    private const string WorkerType = "physics";
    private const uint GetOpListTimeoutInMilliseconds = 100;
    private const uint ClientDataComponentId = 1001;

    private const int ErrorExitStatus = 1;
    private static int Main(string[] args)
    {
        if (args.Length != 3)
        {
            PrintUsage();
            return ErrorExitStatus;
        }

        // Ensure that the generated code assembly is loaded correctly.
        Assembly.Load("GeneratedCode");

        var connectionParameters = new ConnectionParameters
        {
            WorkerType = WorkerType,
            Network =
            {
                ConnectionType = NetworkConnectionType.Tcp
            }
        };

        using (var connection = ConnectWithReceptionist(args[0], Convert.ToUInt16(args[1]), args[2], connectionParameters))
        {
            var view = new View();
            var isConnected = true;
            EntityId ourEntity = new EntityId(1);
            float angle = 0.0f;

            view.OnDisconnect(op =>
            {
                Console.Error.WriteLine("[disconnect] " + op.Reason);
                isConnected = false;
            });

            view.OnLogMessage(op =>
            {
                if (op.Level == LogLevel.Fatal)
                {
                    Console.Error.WriteLine("Fatal error: " + op.Message);
                    Environment.Exit(ErrorExitStatus);
                }
            });

            view.OnCommandRequest(Login.Commands.TakeControl.Metaclass, op =>
            {
                // Assign write ACL of 1001 to the worker ID received.
                connection.SendLogMessage(LogLevel.Info, "Physics", "Assigning ClientData component to " + op.CallerAttributeSet[1]);

                Improbable.EntityAcl.Update aclUpdate = new EntityAcl.Update();
                var currentWriteAcl = view.Entities[ourEntity].Get<EntityAcl>().Value.Get().Value.componentWriteAcl;
                var filteredCallerAttributeSet = new Improbable.Collections.List<string>();           
                filteredCallerAttributeSet.Add(op.CallerAttributeSet[1]);
                var workerAttributeSet = new WorkerAttributeSet(filteredCallerAttributeSet);
                currentWriteAcl[ClientDataComponentId] = new WorkerRequirementSet(new List<WorkerAttributeSet>(new[] {workerAttributeSet}));
                aclUpdate.componentWriteAcl.Set(currentWriteAcl);
                connection.SendComponentUpdate(ourEntity, aclUpdate);
            });

            view.OnCommandResponse(ClientData.Commands.TestCommand.Metaclass, op =>
            {
                if (op.StatusCode == StatusCode.Success)
                {
                    connection.SendLogMessage(LogLevel.Info, "Physics",
                        "Received command response: " + op.Response.Value.sum);
                }
            });

            int tickCount = 0;
            while (isConnected)
            {
                using (var opList = connection.GetOpList(GetOpListTimeoutInMilliseconds))
                {
                    view.Process(opList);

                    // Update position of entity.
                    angle += 0.5f;
                    Position.Update positionUpdate = new Position.Update();
                    positionUpdate.coords.Set(new Coordinates(Math.Sin(angle) * 10, 0.0, Math.Cos(angle) * 10));
                    connection.SendComponentUpdate(Position.Metaclass, ourEntity, positionUpdate);

                    // Sleep for some time.
                    System.Threading.Thread.Sleep(1000);

                    // Send a command every 2 ticks.
                    if (tickCount % 2 == 0)
                    {
                        connection.SendCommandRequest(ClientData.Commands.TestCommand.Metaclass, 
                            ourEntity, new Sample.AddCommandRequest(10, 1.5f), null);
                    }
                    tickCount++;
                }
            }
        }

        // This means we forcefully disconnected
        return ErrorExitStatus;
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Usage: mono Physics.exe <hostname> <port> <worker_id>");
        Console.WriteLine("Connects to SpatialOS");
        Console.WriteLine("    <hostname>      - hostname of the receptionist to connect to.");
        Console.WriteLine("    <port>          - port to use");
        Console.WriteLine("    <worker_id>     - name of the worker assigned by SpatialOS.");
    }

    private static Connection ConnectWithReceptionist(string hostname, ushort port,
        string workerId, ConnectionParameters connectionParameters)
    {
        Connection connection;

        connectionParameters.Network.UseExternalIp = false;

        using (var future = Connection.ConnectAsync(hostname, port, workerId, connectionParameters))
        {
            connection = future.Get();
        }

        Console.WriteLine("Successfully connected using the Receptionist");

        return connection;
    }
}