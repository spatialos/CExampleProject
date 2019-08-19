#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define POSITION_COMPONENT_ID 54
#define LOGIN_COMPONENT_ID 1000
#define CLIENTDATA_COMPONENT_ID 1001

char* GenerateWorkerId(char* worker_id_prefix) {
  /* Calculate buffer size. */
  char* fmt = "%s%d";
  int id = rand() % 10000000;
  int size = snprintf(NULL, 0, fmt, worker_id_prefix, id);

  /* Format string. */
  char* worker_id = malloc(sizeof(char) * (size + 1));
  sprintf(worker_id, fmt, worker_id_prefix, id);
  return worker_id;
}

/** Op handler functions. */
void OnLogMessage(const Worker_LogMessageOp* op) {
  printf("log: %s\n", op->message);
}

void OnDisconnect(const Worker_DisconnectOp* op) {
  printf("disconnected. reason: %s\n", op->reason);
}

void OnEntityQueryResponse(const Worker_EntityQueryResponseOp* op) {
  printf("entity query result: %d entities. Status: %d. Results: %p\n", op->result_count,
         op->status_code, (void*)op->results);
  if (op->results) {
    for (uint32_t i = 0; i < op->result_count; ++i) {
      const Worker_Entity* entity = &op->results[i];
      printf("- entity %" PRId64 " with %d components", entity->entity_id, entity->component_count);
      for (uint32_t k = 0; k < entity->component_count; ++k) {
        if (entity->components[k].component_id == POSITION_COMPONENT_ID) {
          Schema_Object* coords_object =
              Schema_GetObject(Schema_GetComponentDataFields(entity->components[k].schema_type), 1);
          double x = Schema_GetDouble(coords_object, 1);
          double y = Schema_GetDouble(coords_object, 2);
          double z = Schema_GetDouble(coords_object, 3);
          printf(": Position: (%f, %f, %f)\n", x, y, z);
        }
      }
      if (entity->component_count == 0) {
        printf("\n");
      }
    }
  }
}

void OnAddComponent(const Worker_AddComponentOp* op) {
  printf("received add component op (entity: %" PRId64 ", component: %d)\n", op->entity_id,
         op->data.component_id);

  if (op->data.component_id == POSITION_COMPONENT_ID) {
    /* Received position data */
    double x, y, z;
    Schema_Object* coords_object =
        Schema_GetObject(Schema_GetComponentDataFields(op->data.schema_type), 1);
    x = Schema_GetDouble(coords_object, 1);
    y = Schema_GetDouble(coords_object, 2);
    z = Schema_GetDouble(coords_object, 3);
    printf("received improbable.Position initial data: (%f, %f, %f)\n", x, y, z);
  }
}

void OnComponentUpdate(const Worker_ComponentUpdateOp* op) {
  printf("received component update op (entity: %" PRId64 ", component: %d)\n", op->entity_id,
         op->update.component_id);

  if (op->update.component_id == POSITION_COMPONENT_ID) {
    /* Received position update */
    Schema_Object* coords_object =
        Schema_GetObject(Schema_GetComponentUpdateFields(op->update.schema_type), 1);
    double x = Schema_GetDouble(coords_object, 1);
    double y = Schema_GetDouble(coords_object, 2);
    double z = Schema_GetDouble(coords_object, 3);
    printf("received improbable.Position update: (%f, %f, %f)\n", x, y, z);
  }
}

void OnCommandRequest(Worker_Connection* connection, const Worker_CommandRequestOp* op) {
  Schema_FieldId command_index = op->request.command_index;
  printf("received command request (entity: %" PRId64 ", component: %d, command: %d).\n",
         op->entity_id, op->request.component_id, command_index);

  if (op->request.component_id == CLIENTDATA_COMPONENT_ID && command_index == 1) {
    Schema_Object* payload = Schema_GetCommandRequestObject(op->request.schema_type);
    int payload1 = Schema_GetInt32(payload, 1);
    float payload2 = Schema_GetFloat(payload, 2);

    float sum = payload1 + payload2;
    Worker_CommandResponse response = {0};
    response.command_index = command_index;
    response.component_id = op->request.component_id;
    response.schema_type = Schema_CreateCommandResponse();
    Schema_Object* response_object = Schema_GetCommandResponseObject(response.schema_type);
    Schema_AddFloat(response_object, 1, sum);
    Worker_Connection_SendCommandResponse(connection, op->request_id, &response);

    printf("sending command response. Sum: %f\n", sum);
  }
}

int main(int argc, char** argv) {
  srand(time(NULL));

  if (argc != 4) {
    printf("Usage: %s <hostname> <port> <worker_id>\n", argv[0]);
    printf("Connects to SpatialOS\n");
    printf("    <hostname>      - hostname of the receptionist to connect to.\n");
    printf("    <port>          - port to use\n");
    printf("    <worker_id>     - name of the worker assigned by SpatialOS. A random prefix will be added to it to ensure uniqueness.\n");
    return EXIT_FAILURE;
  }

  /* Default vtable. This enables schema objects to be passed through the C API directly to us. */
  Worker_ComponentVtable default_vtable = {0};

  /* Generate worker ID. */
  char* worker_id = GenerateWorkerId(argv[3]);

  /* Connect to SpatialOS. */
  Worker_ConnectionParameters params = Worker_DefaultConnectionParameters();
  params.worker_type = "client_direct";
  params.network.tcp.multiplex_level = 4;
  params.default_component_vtable = &default_vtable;
  Worker_ConnectionFuture* connection_future =
      Worker_ConnectAsync(argv[1], (uint16_t)atoi(argv[2]), worker_id, &params);
  Worker_Connection* connection = Worker_ConnectionFuture_Get(connection_future, NULL);
  Worker_ConnectionFuture_Destroy(connection_future);
  free(worker_id);

  /* Send a test message. */
  Worker_LogMessage message = {WORKER_LOG_LEVEL_WARN, "Client", "Connected successfully", NULL};
  Worker_Connection_SendLogMessage(connection, &message);

  /* Send an entity query. */
  Worker_EntityQuery query;
  query.constraint.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
  query.constraint.constraint.entity_id_constraint.entity_id = 1;
  query.result_type = WORKER_RESULT_TYPE_SNAPSHOT;
  query.snapshot_result_type_component_id_count = 1;
  Worker_ComponentId position_component_id = POSITION_COMPONENT_ID;
  query.snapshot_result_type_component_ids = &position_component_id;
  Worker_Connection_SendEntityQueryRequest(connection, &query, NULL);

  /* Take control of the entity. */
  Worker_CommandRequest command_request;
  memset(&command_request, 0, sizeof(command_request));
  command_request.component_id = LOGIN_COMPONENT_ID;
  command_request.command_index = 1;
  command_request.schema_type = Schema_CreateCommandRequest();
  Worker_CommandParameters command_parameters;
  command_parameters.allow_short_circuit = 0;
  Worker_Connection_SendCommandRequest(connection, 1, &command_request, NULL, &command_parameters);

  /* Main loop. */
  while (1) {
    Worker_OpList* op_list = Worker_Connection_GetOpList(connection, 0);
    for (size_t i = 0; i < op_list->op_count; ++i) {
      Worker_Op* op = &op_list->ops[i];
      switch (op->op_type) {
      case WORKER_OP_TYPE_DISCONNECT:
        OnDisconnect(&op->op.disconnect);
        break;
      case WORKER_OP_TYPE_LOG_MESSAGE:
        OnLogMessage(&op->op.log_message);
        break;
      case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
        OnEntityQueryResponse(&op->op.entity_query_response);
        break;
      case WORKER_OP_TYPE_ADD_COMPONENT:
        OnAddComponent(&op->op.add_component);
        break;
      case WORKER_OP_TYPE_COMPONENT_UPDATE:
        OnComponentUpdate(&op->op.component_update);
        break;
      case WORKER_OP_TYPE_COMMAND_REQUEST:
        OnCommandRequest(connection, &op->op.command_request);
        break;
      default:
        break;
      }
    }
    Worker_OpList_Destroy(op_list);
  }

  Worker_Connection_Destroy(connection);
}
