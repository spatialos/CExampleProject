#include "schema_types.h"
#include <stdlib.h>
#include <string.h>

GenericCommandObject* CreateCommandObject(Schema_FieldId command_index, void* data) {
  GenericCommandObject* command_object =
      (GenericCommandObject*)malloc(sizeof(GenericCommandObject));
  command_object->command_index = command_index;
  command_object->data = data;
  return command_object;
}

void Improbable_Position_ComponentDataFree(Worker_ComponentId component_id, void* user_data,
                                           Worker_ComponentDataHandle* handle) {
  free(handle);
}

Worker_ComponentDataHandle*
Improbable_Position_ComponentDataCopy(Worker_ComponentId component_id, void* user_data,
                                      Worker_ComponentDataHandle* handle) {
  Improbable_Position* data = (Improbable_Position*)handle;
  Improbable_Position* new_data = (Improbable_Position*)calloc(1, sizeof(Improbable_Position));
  *new_data = *data;
  return new_data;
}

uint8_t Improbable_Position_ComponentDataDeserialize(Worker_ComponentId component_id,
                                                     void* user_data, Schema_ComponentData* source,
                                                     Worker_ComponentDataHandle** handle_out) {
  Improbable_Position* new_data = (Improbable_Position*)calloc(1, sizeof(Improbable_Position));
  Schema_Object* fields = Schema_GetComponentDataFields(source);
  Schema_Object* coords_object = Schema_GetObject(fields, 1);
  new_data->coords.x = Schema_GetDouble(coords_object, 1);
  new_data->coords.y = Schema_GetDouble(coords_object, 2);
  new_data->coords.z = Schema_GetDouble(coords_object, 3);
  *handle_out = new_data;
  return 1;
}

void Improbable_Position_ComponentDataSerialize(Worker_ComponentId component_id, void* user_data,
                                                Worker_ComponentDataHandle* handle,
                                                Schema_ComponentData** target_out) {
  Improbable_Position* data = (Improbable_Position*)handle;
  *target_out = Schema_CreateComponentData();
  Schema_Object* fields = Schema_GetComponentDataFields(*target_out);
  Schema_Object* coords_object = Schema_AddObject(fields, 1);
  Schema_AddDouble(coords_object, 1, data->coords.x);
  Schema_AddDouble(coords_object, 2, data->coords.y);
  Schema_AddDouble(coords_object, 3, data->coords.z);
}

void Improbable_Position_ComponentUpdateFree(Worker_ComponentId component_id, void* user_data,
                                             Worker_ComponentUpdateHandle* handle) {
  Improbable_PositionUpdate* update = (Improbable_PositionUpdate*)handle;
  free(update->coords);
  free(update);
}

Worker_ComponentUpdateHandle*
Improbable_Position_ComponentUpdateCopy(Worker_ComponentId component_id, void* user_data,
                                        Worker_ComponentUpdateHandle* handle) {
  Improbable_PositionUpdate* update = (Improbable_PositionUpdate*)handle;
  Improbable_PositionUpdate* new_update =
      (Improbable_PositionUpdate*)calloc(1, sizeof(Improbable_PositionUpdate));
  if (update->coords) {
    new_update->coords = (Improbable_Coords*)malloc(sizeof(Improbable_Coords));
    *new_update->coords = *update->coords;
  }
  return new_update;
}

uint8_t Improbable_Position_ComponentUpdateDeserialize(Worker_ComponentId component_id,
                                                       void* user_data,
                                                       Schema_ComponentUpdate* source,
                                                       Worker_ComponentUpdateHandle** handle_out) {
  Improbable_PositionUpdate* new_update =
      (Improbable_PositionUpdate*)calloc(1, sizeof(Improbable_PositionUpdate));
  Schema_Object* fields = Schema_GetComponentUpdateFields(source);
  if (Schema_GetObjectCount(fields, 1) == 1) {
    new_update->coords = (Improbable_Coords*)malloc(sizeof(Improbable_Coords));
    Schema_Object* coords_object = Schema_GetObject(fields, 1);
    new_update->coords->x = Schema_GetDouble(coords_object, 1);
    new_update->coords->y = Schema_GetDouble(coords_object, 2);
    new_update->coords->z = Schema_GetDouble(coords_object, 3);
  }
  *handle_out = new_update;
  return 1;
}

void Improbable_Position_ComponentUpdateSerialize(Worker_ComponentId component_id, void* user_data,
                                                  Worker_ComponentUpdateHandle* handle,
                                                  Schema_ComponentUpdate** target_out) {
  Improbable_PositionUpdate* update = (Improbable_PositionUpdate*)handle;
  *target_out = Schema_CreateComponentUpdate();
  Schema_Object* fields = Schema_GetComponentUpdateFields(*target_out);
  if (update->coords) {
    update->coords = (Improbable_Coords*)malloc(sizeof(Improbable_Coords));
    Schema_Object* coords_object = Schema_AddObject(fields, 1);
    Schema_AddDouble(coords_object, 1, update->coords->x);
    Schema_AddDouble(coords_object, 2, update->coords->y);
    Schema_AddDouble(coords_object, 3, update->coords->z);
  }
}

void Sample_Login_CommandRequestFree(Worker_ComponentId component_id,
                                     Worker_CommandIndex command_index, void* user_data,
                                     Worker_CommandRequestHandle* handle) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  free(command_object->data);
  free(command_object);
}

Worker_CommandRequestHandle* Sample_Login_CommandRequestCopy(Worker_ComponentId component_id,
                                                             Worker_CommandIndex command_index,
                                                             void* user_data,
                                                             Worker_CommandRequestHandle* handle) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  if (command_object->command_index == 1) {
    Sample_Login_TakeControl_Request* data =
        (Sample_Login_TakeControl_Request*)command_object->data;
    Sample_Login_TakeControl_Request* new_data =
        (Sample_Login_TakeControl_Request*)calloc(1, sizeof(Sample_Login_TakeControl_Request));
    (void)data;
    return CreateCommandObject(command_object->command_index, new_data);
  } else {
    /* We don't handle any other command index for this component. */
    return NULL;
  }
}

uint8_t Sample_Login_CommandRequestDeserialize(Worker_ComponentId component_id,
                                               Worker_CommandIndex command_index, void* user_data,
                                               Schema_CommandRequest* source,
                                               Worker_CommandRequestHandle** handle_out) {
  if (command_index == 1) {
    Sample_Login_TakeControl_Request* new_data =
        (Sample_Login_TakeControl_Request*)calloc(1, sizeof(Sample_Login_TakeControl_Request));
    *handle_out = CreateCommandObject(1, new_data);
    return 1;
  } else {
    /* We don't handle any other command index for this component. */
    *handle_out = NULL;
    return 0;
  }
}

void Sample_Login_CommandRequestSerialize(Worker_ComponentId component_id,
                                          Worker_CommandIndex command_index, void* user_data,
                                          Worker_CommandRequestHandle* handle,
                                          Schema_CommandRequest** target_out) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  if (command_object->command_index == 1) {
    Sample_Login_TakeControl_Request* data =
        (Sample_Login_TakeControl_Request*)command_object->data;
    *target_out = Schema_CreateCommandRequest();
    (void)data;
  } else {
    /* We don't handle any other command index for this component. */
    *target_out = NULL;
  }
}

void Sample_Login_CommandResponseFree(Worker_ComponentId component_id,
                                      Worker_CommandIndex command_index, void* user_data,
                                      Worker_CommandResponseHandle* handle) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  free(command_object->data);
  free(command_object);
}

Worker_CommandResponseHandle*
Sample_Login_CommandResponseCopy(Worker_ComponentId component_id, Worker_CommandIndex command_index,
                                 void* user_data, Worker_CommandResponseHandle* handle) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  if (command_object->command_index == 1) {
    Sample_Login_TakeControl_Response* data =
        (Sample_Login_TakeControl_Response*)command_object->data;
    Sample_Login_TakeControl_Response* new_data =
        (Sample_Login_TakeControl_Response*)calloc(1, sizeof(Sample_Login_TakeControl_Response));
    (void)data;
    return CreateCommandObject(command_object->command_index, new_data);
  } else {
    /* We don't handle any other command index for this component. */
    return NULL;
  }
}

uint8_t Sample_Login_CommandResponseDeserialize(Worker_ComponentId component_id,
                                                Worker_CommandIndex command_index, void* user_data,
                                                Schema_CommandResponse* source,
                                                Worker_CommandResponseHandle** handle_out) {
  if (command_index == 1) {
    Sample_Login_TakeControl_Response* new_data =
        (Sample_Login_TakeControl_Response*)calloc(1, sizeof(Sample_Login_TakeControl_Response));
    *handle_out = CreateCommandObject(1, new_data);
    return 1;
  } else {
    /* We don't handle any other command index for this component. */
    *handle_out = NULL;
    return 0;
  }
}

void Sample_Login_CommandResponseSerialize(Worker_ComponentId component_id,
                                           Worker_CommandIndex command_index, void* user_data,
                                           Worker_CommandResponseHandle* handle,
                                           Schema_CommandResponse** target_out) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  if (command_object->command_index == 1) {
    Sample_Login_TakeControl_Response* data =
        (Sample_Login_TakeControl_Response*)command_object->data;
    *target_out = Schema_CreateCommandResponse();
    (void)data;
  } else {
    /* We don't handle any other command index for this component. */
    *target_out = NULL;
  }
}

void Sample_ClientData_CommandRequestFree(Worker_ComponentId component_id,
                                          Worker_CommandIndex command_index, void* user_data,
                                          Worker_CommandRequestHandle* handle) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  free(command_object->data);
  free(command_object);
}

Worker_CommandRequestHandle*
Sample_ClientData_CommandRequestCopy(Worker_ComponentId component_id,
                                     Worker_CommandIndex command_index, void* user_data,
                                     Worker_CommandRequestHandle* handle) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  if (command_object->command_index == 1) {
    Sample_ClientData_TestCommand_Request* data =
        (Sample_ClientData_TestCommand_Request*)command_object->data;
    Sample_ClientData_TestCommand_Request* new_data =
        (Sample_ClientData_TestCommand_Request*)calloc(
            1, sizeof(Sample_ClientData_TestCommand_Request));
    *new_data = *data;
    return CreateCommandObject(command_object->command_index, new_data);
  } else {
    /* We don't handle any other command index for this component. */
    return NULL;
  }
}

uint8_t Sample_ClientData_CommandRequestDeserialize(Worker_ComponentId component_id,
                                                    Worker_CommandIndex command_index,
                                                    void* user_data, Schema_CommandRequest* source,
                                                    Worker_CommandRequestHandle** handle_out) {
  if (command_index == 1) {
    Sample_ClientData_TestCommand_Request* new_data =
        (Sample_ClientData_TestCommand_Request*)calloc(
            1, sizeof(Sample_ClientData_TestCommand_Request));
    Schema_Object* fields = Schema_GetCommandRequestObject(source);
    new_data->payload1 = Schema_GetInt32(fields, 1);
    new_data->payload2 = Schema_GetFloat(fields, 2);
    *handle_out = CreateCommandObject(1, new_data);
    return 1;
  } else {
    /* We don't handle any other command index for this component. */
    *handle_out = NULL;
    return 0;
  }
}

void Sample_ClientData_CommandRequestSerialize(Worker_ComponentId component_id,
                                               Worker_CommandIndex command_index, void* user_data,
                                               Worker_CommandRequestHandle* handle,
                                               Schema_CommandRequest** target_out) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  if (command_object->command_index == 1) {
    Sample_ClientData_TestCommand_Request* data =
        (Sample_ClientData_TestCommand_Request*)command_object->data;
    *target_out = Schema_CreateCommandRequest();
    Schema_Object* fields = Schema_GetCommandRequestObject(*target_out);
    Schema_AddInt32(fields, 1, data->payload1);
    Schema_AddFloat(fields, 2, data->payload2);
  } else {
    /* We don't handle any other command index for this component. */
    *target_out = NULL;
  }
}

void Sample_ClientData_CommandResponseFree(Worker_ComponentId component_id,
                                           Worker_CommandIndex command_index, void* user_data,
                                           Worker_CommandResponseHandle* handle) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  free(command_object->data);
  free(command_object);
}

Worker_CommandResponseHandle*
Sample_ClientData_CommandResponseCopy(Worker_ComponentId component_id,
                                      Worker_CommandIndex command_index, void* user_data,
                                      Worker_CommandResponseHandle* handle) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  if (command_object->command_index == 1) {
    Sample_ClientData_TestCommand_Response* data =
        (Sample_ClientData_TestCommand_Response*)command_object->data;
    Sample_ClientData_TestCommand_Response* new_data =
        (Sample_ClientData_TestCommand_Response*)calloc(
            1, sizeof(Sample_ClientData_TestCommand_Response));
    *new_data = *data;
    return CreateCommandObject(command_object->command_index, new_data);
  } else {
    /* We don't handle any other command index for this component. */
    return NULL;
  }
}

uint8_t Sample_ClientData_CommandResponseDeserialize(Worker_ComponentId component_id,
                                                     Worker_CommandIndex command_index,
                                                     void* user_data,
                                                     Schema_CommandResponse* source,
                                                     Worker_CommandResponseHandle** handle_out) {
  if (command_index == 1) {
    Sample_ClientData_TestCommand_Response* new_data =
        (Sample_ClientData_TestCommand_Response*)calloc(
            1, sizeof(Sample_ClientData_TestCommand_Response));
    Schema_Object* fields = Schema_GetCommandResponseObject(source);
    new_data->sum = Schema_GetFloat(fields, 1);
    *handle_out = CreateCommandObject(1, new_data);
    return 1;
  } else {
    /* We don't handle any other command index for this component. */
    *handle_out = NULL;
    return 0;
  }
}

void Sample_ClientData_CommandResponseSerialize(Worker_ComponentId component_id,
                                                Worker_CommandIndex command_index, void* user_data,
                                                Worker_CommandResponseHandle* handle,
                                                Schema_CommandResponse** target_out) {
  GenericCommandObject* command_object = (GenericCommandObject*)handle;
  if (command_object->command_index == 1) {
    Sample_ClientData_TestCommand_Response* data =
        (Sample_ClientData_TestCommand_Response*)command_object->data;
    *target_out = Schema_CreateCommandResponse();
    Schema_Object* fields = Schema_GetCommandResponseObject(*target_out);
    Schema_AddFloat(fields, 1, data->sum);
  } else {
    /* We don't handle any other command index for this component. */
    *target_out = NULL;
  }
}

void Sample_ClientData_ComponentDataFree(Worker_ComponentId component_id, void* user_data,
                                         Worker_ComponentDataHandle* handle) {
  free(handle);
}

Worker_ComponentDataHandle*
Sample_ClientData_ComponentDataCopy(Worker_ComponentId component_id, void* user_data,
                                    Worker_ComponentDataHandle* handle) {
  Sample_ClientData* data = (Sample_ClientData*)handle;
  Sample_ClientData* new_data = (Sample_ClientData*)calloc(1, sizeof(Sample_ClientData));
  *new_data = *data;
  return new_data;
}

uint8_t Sample_ClientData_ComponentDataDeserialize(Worker_ComponentId component_id, void* user_data,
                                                   Schema_ComponentData* source,
                                                   Worker_ComponentDataHandle** handle_out) {
  Sample_ClientData* new_data = (Sample_ClientData*)calloc(1, sizeof(Sample_ClientData));
  Schema_Object* fields = Schema_GetComponentDataFields(source);
  new_data->input_state = Schema_GetFloat(fields, 1);
  *handle_out = new_data;
  return 1;
}

void Sample_ClientData_ComponentDataSerialize(Worker_ComponentId component_id, void* user_data,
                                              Worker_ComponentDataHandle* handle,
                                              Schema_ComponentData** target_out) {
  Sample_ClientData* data = (Sample_ClientData*)handle;
  *target_out = Schema_CreateComponentData();
  Schema_Object* fields = Schema_GetComponentDataFields(*target_out);
  Schema_AddFloat(fields, 1, data->input_state);
}

void Sample_ClientData_ComponentUpdateFree(Worker_ComponentId component_id, void* user_data,
                                           Worker_ComponentUpdateHandle* handle) {
  Sample_ClientDataUpdate* update = (Sample_ClientDataUpdate*)handle;
  free(update->input_state);
  free(update);
}

Worker_ComponentUpdateHandle*
Sample_ClientData_ComponentUpdateCopy(Worker_ComponentId component_id, void* user_data,
                                      Worker_ComponentUpdateHandle* handle) {
  Sample_ClientDataUpdate* update = (Sample_ClientDataUpdate*)handle;
  Sample_ClientDataUpdate* new_update =
      (Sample_ClientDataUpdate*)calloc(1, sizeof(Sample_ClientDataUpdate));
  if (update->input_state) {
    new_update->input_state = (float*)malloc(sizeof(float));
    *new_update->input_state = *update->input_state;
  }
  return new_update;
}

uint8_t Sample_ClientData_ComponentUpdateDeserialize(Worker_ComponentId component_id,
                                                     void* user_data,
                                                     Schema_ComponentUpdate* source,
                                                     Worker_ComponentUpdateHandle** handle_out) {
  Sample_ClientDataUpdate* new_update =
      (Sample_ClientDataUpdate*)calloc(1, sizeof(Sample_ClientDataUpdate));
  Schema_Object* fields = Schema_GetComponentUpdateFields(source);
  if (Schema_GetFloatCount(fields, 1) == 1) {
    new_update->input_state = (float*)malloc(sizeof(float));
    *new_update->input_state = Schema_GetFloat(fields, 1);
  }
  *handle_out = new_update;
  return 1;
}

void Sample_ClientData_ComponentUpdateSerialize(Worker_ComponentId component_id, void* user_data,
                                                Worker_ComponentUpdateHandle* handle,
                                                Schema_ComponentUpdate** target_out) {
  Sample_ClientDataUpdate* update = (Sample_ClientDataUpdate*)handle;
  *target_out = Schema_CreateComponentUpdate();
  Schema_Object* fields = Schema_GetComponentUpdateFields(*target_out);
  if (update->input_state) {
    Schema_AddFloat(fields, 1, *update->input_state);
  }
}
