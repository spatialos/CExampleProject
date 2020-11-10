#include "schema_types.h"
#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

uint8_t LogFilter(void* user_data_unused, uint32_t categories, Worker_LogLevel level) {
  (void)user_data_unused;
  return level >= WORKER_LOG_LEVEL_WARN ||
          (level >= WORKER_LOG_LEVEL_INFO && categories & WORKER_LOG_CATEGORY_LOGIN)
      ? 1u
      : 0u;
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
      for (uint32_t i = 0; i < entity->component_count; ++i) {
        if (entity->components[i].component_id == POSITION_COMPONENT_ID) {
          Improbable_Position* position = (Improbable_Position*)entity->components[i].user_handle;
          printf(": Position: (%f, %f, %f)\n", position->coords.x, position->coords.y,
                 position->coords.z);
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
    Improbable_Position* position = (Improbable_Position*)op->data.user_handle;
    printf("received improbable.Position initial data: (%f, %f, %f)\n", position->coords.x,
           position->coords.y, position->coords.z);
  }
}

void OnComponentUpdate(const Worker_ComponentUpdateOp* op) {
  printf("received component update op (entity: %" PRId64 ", component: %d)\n", op->entity_id,
         op->update.component_id);

  if (op->update.component_id == POSITION_COMPONENT_ID) {
    /* Received position update */
    Improbable_PositionUpdate* position = (Improbable_PositionUpdate*)op->update.user_handle;
    if (position->coords) {
      printf("received improbable.Position update: (%f, %f, %f)\n", position->coords->x,
             position->coords->y, position->coords->z);
    }
  }
}

void OnCommandRequest(Worker_Connection* connection, const Worker_CommandRequestOp* op) {
  Schema_FieldId command_index = op->request.command_index;
  printf("received command request (entity: %" PRId64 ", component: %d, command: %d).\n",
         op->entity_id, op->request.component_id, command_index);

  if (op->request.component_id == CLIENTDATA_COMPONENT_ID && command_index == 1) {
    Sample_AddCommandRequest* request =
        (Sample_AddCommandRequest*)((GenericCommandObject*)op->request.user_handle)->data;

    Sample_AddCommandResponse response_data;
    response_data.sum = request->payload1 + request->payload2;

    Worker_CommandResponse response = {0};
    response.command_index = command_index;
    response.component_id = op->request.component_id;
    response.user_handle = CreateCommandObject(1, &response_data);
    Worker_Connection_SendCommandResponse(connection, op->request_id, &response);
    free(response.user_handle);

    printf("sending command response. Sum: %f\n", response_data.sum);
  }
}

int main(int argc, char** argv) {
  srand(time(NULL));

  if (argc != 4) {
    printf("Usage: %s <hostname> <port> <worker_id>\n", argv[0]);
    printf("Connects to SpatialOS\n");
    printf("    <hostname>      - hostname of the receptionist to connect to.\n");
    printf("    <port>          - port to use\n");
    printf("    <worker_id>     - name of the worker assigned by SpatialOS.\n");
    return EXIT_FAILURE;
  }

  /* Set up component vtables. */
  Worker_ComponentVtable component_vtables[3] = {0};

  /* Generate worker ID. */
  char* worker_id = GenerateWorkerId(argv[3]);

  /* improbable.Position */
  component_vtables[0].component_id = POSITION_COMPONENT_ID;
  component_vtables[0].component_data_free = &Improbable_Position_ComponentDataFree;
  component_vtables[0].component_data_copy = &Improbable_Position_ComponentDataCopy;
  component_vtables[0].component_data_deserialize = &Improbable_Position_ComponentDataDeserialize;
  component_vtables[0].component_data_serialize = &Improbable_Position_ComponentDataSerialize;
  component_vtables[0].component_update_free = &Improbable_Position_ComponentUpdateFree;
  component_vtables[0].component_update_copy = &Improbable_Position_ComponentUpdateCopy;
  component_vtables[0].component_update_deserialize =
      &Improbable_Position_ComponentUpdateDeserialize;
  component_vtables[0].component_update_serialize = &Improbable_Position_ComponentUpdateSerialize;

  /* sample.Login */
  component_vtables[1].component_id = LOGIN_COMPONENT_ID;
  component_vtables[1].command_request_free = &Sample_Login_CommandRequestFree;
  component_vtables[1].command_request_copy = &Sample_Login_CommandRequestCopy;
  component_vtables[1].command_request_deserialize = &Sample_Login_CommandRequestDeserialize;
  component_vtables[1].command_request_serialize = &Sample_Login_CommandRequestSerialize;
  component_vtables[1].command_response_free = &Sample_Login_CommandResponseFree;
  component_vtables[1].command_response_copy = &Sample_Login_CommandResponseCopy;
  component_vtables[1].command_response_deserialize = &Sample_Login_CommandResponseDeserialize;
  component_vtables[1].command_response_serialize = &Sample_Login_CommandResponseSerialize;

  /* sample.ClientData */
  component_vtables[2].component_id = CLIENTDATA_COMPONENT_ID;
  component_vtables[2].command_request_free = &Sample_ClientData_CommandRequestFree;
  component_vtables[2].command_request_copy = &Sample_ClientData_CommandRequestCopy;
  component_vtables[2].command_request_deserialize = &Sample_ClientData_CommandRequestDeserialize;
  component_vtables[2].command_request_serialize = &Sample_ClientData_CommandRequestSerialize;
  component_vtables[2].command_response_free = &Sample_ClientData_CommandResponseFree;
  component_vtables[2].command_response_copy = &Sample_ClientData_CommandResponseCopy;
  component_vtables[2].command_response_deserialize = &Sample_ClientData_CommandResponseDeserialize;
  component_vtables[2].command_response_serialize = &Sample_ClientData_CommandResponseSerialize;
  component_vtables[2].component_data_free = &Sample_ClientData_ComponentDataFree;
  component_vtables[2].component_data_copy = &Sample_ClientData_ComponentDataCopy;
  component_vtables[2].component_data_deserialize = &Sample_ClientData_ComponentDataDeserialize;
  component_vtables[2].component_data_serialize = &Sample_ClientData_ComponentDataSerialize;
  component_vtables[2].component_update_free = &Sample_ClientData_ComponentUpdateFree;
  component_vtables[2].component_update_copy = &Sample_ClientData_ComponentUpdateCopy;
  component_vtables[2].component_update_deserialize = &Sample_ClientData_ComponentUpdateDeserialize;
  component_vtables[2].component_update_serialize = &Sample_ClientData_ComponentUpdateSerialize;

  /* Connect to SpatialOS. */
  Worker_ConnectionParameters params = Worker_DefaultConnectionParameters();
  params.worker_type = "client_vtable";
  params.network.connection_type = WORKER_NETWORK_CONNECTION_TYPE_KCP;
  params.network.kcp.security_type = WORKER_NETWORK_SECURITY_TYPE_INSECURE;
  params.component_vtable_count = sizeof(component_vtables) / sizeof(component_vtables[0]);
  params.component_vtables = component_vtables;

  Worker_LogsinkParameters logsink_params = {0};
  logsink_params.logsink_type = WORKER_LOGSINK_TYPE_STDOUT;
  logsink_params.filter_parameters.callback = &LogFilter;
  params.logsink_count = 1u;
  params.logsinks = &logsink_params;
  params.enable_logging_at_startup = 1u;

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
  query.snapshot_result_type_component_id_count = 1;
  Worker_ComponentId position_component_id = POSITION_COMPONENT_ID;
  query.snapshot_result_type_component_ids = &position_component_id;
  query.snapshot_result_type_component_set_id_count = 0;
  query.snapshot_result_type_component_set_ids = NULL;
  Worker_Connection_SendEntityQueryRequest(connection, &query, NULL);

  /* Take control of the entity. */
  Sample_Login_TakeControl_Request take_control_request;

  Worker_CommandRequest command_request;
  memset(&command_request, 0, sizeof(command_request));
  command_request.component_id = LOGIN_COMPONENT_ID;
  command_request.command_index = 1;
  command_request.user_handle = CreateCommandObject(1, &take_control_request);
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
