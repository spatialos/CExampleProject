#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#define POSITION_COMPONENT_ID 54
#define LOGIN_COMPONENT_ID 1000
#define CLIENTDATA_COMPONENT_ID 1001

/*
 * A helper struct which stores a command index and command request / response data. Used to store
 * the command index when reading a Worker_CommandRequestHandle or Worker_CommandResponseHandle.
 */
typedef struct {
  Schema_FieldId command_index;
  void* data;
} GenericCommandObject;

GenericCommandObject* CreateCommandObject(Schema_FieldId command_index, void* data);

/*
 * Data types used for serialization. These would usually be generated.
 */

/* improbable.Position */
typedef struct {
  double x;
  double y;
  double z;
} Improbable_Coords;
typedef struct { Improbable_Coords coords; } Improbable_Position;

typedef struct { Improbable_Coords* coords; } Improbable_PositionUpdate;

/* sample.Login */
typedef struct {
  /* C requires structs to not be empty. */
  void* __unused__;
} Sample_Empty;

typedef Sample_Empty Sample_Login_TakeControl_Request;
typedef Sample_Empty Sample_Login_TakeControl_Response;

/* sample.ClientData */
typedef struct { float input_state; } Sample_ClientData;

typedef struct { float* input_state; } Sample_ClientDataUpdate;

typedef struct {
  int32_t payload1;
  float payload2;
} Sample_AddCommandRequest;

typedef struct { float sum; } Sample_AddCommandResponse;

typedef Sample_AddCommandRequest Sample_ClientData_TestCommand_Request;
typedef Sample_AddCommandResponse Sample_ClientData_TestCommand_Response;

/*
 * Vtable functions.
 */

/* improbable.Position */
void Improbable_Position_ComponentDataFree(Worker_ComponentId component_id, void* user_data,
                                           Worker_ComponentDataHandle* handle);
Worker_ComponentDataHandle*
Improbable_Position_ComponentDataCopy(Worker_ComponentId component_id, void* user_data,
                                      Worker_ComponentDataHandle* handle);
uint8_t Improbable_Position_ComponentDataDeserialize(Worker_ComponentId component_id,
                                                     void* user_data, Schema_ComponentData* source,
                                                     Worker_ComponentDataHandle** handle_out);
void Improbable_Position_ComponentDataSerialize(Worker_ComponentId component_id, void* user_data,
                                                Worker_ComponentDataHandle* handle,
                                                Schema_ComponentData** target_out);

void Improbable_Position_ComponentUpdateFree(Worker_ComponentId component_id, void* user_data,
                                             Worker_ComponentUpdateHandle* handle);
Worker_ComponentUpdateHandle*
Improbable_Position_ComponentUpdateCopy(Worker_ComponentId component_id, void* user_data,
                                        Worker_ComponentUpdateHandle* handle);
uint8_t Improbable_Position_ComponentUpdateDeserialize(Worker_ComponentId component_id,
                                                       void* user_data,
                                                       Schema_ComponentUpdate* source,
                                                       Worker_ComponentUpdateHandle** handle_out);
void Improbable_Position_ComponentUpdateSerialize(Worker_ComponentId component_id, void* user_data,
                                                  Worker_ComponentUpdateHandle* handle,
                                                  Schema_ComponentUpdate** target_out);

/* sample.ClientData */
void Sample_Login_CommandRequestFree(Worker_ComponentId component_id, void* user_data,
                                     Worker_CommandRequestHandle* handle);
Worker_CommandRequestHandle* Sample_Login_CommandRequestCopy(Worker_ComponentId component_id,
                                                             void* user_data,
                                                             Worker_CommandRequestHandle* handle);
uint8_t Sample_Login_CommandRequestDeserialize(Worker_ComponentId component_id, void* user_data,
                                               Schema_CommandRequest* source,
                                               Worker_CommandRequestHandle** handle_out);
void Sample_Login_CommandRequestSerialize(Worker_ComponentId component_id, void* user_data,
                                          Worker_CommandRequestHandle* handle,
                                          Schema_CommandRequest** target_out);

void Sample_Login_CommandResponseFree(Worker_ComponentId component_id, void* user_data,
                                      Worker_CommandResponseHandle* handle);
Worker_CommandResponseHandle*
Sample_Login_CommandResponseCopy(Worker_ComponentId component_id, void* user_data,
                                 Worker_CommandResponseHandle* handle);
uint8_t Sample_Login_CommandResponseDeserialize(Worker_ComponentId component_id, void* user_data,
                                                Schema_CommandResponse* source,
                                                Worker_CommandResponseHandle** handle_out);
void Sample_Login_CommandResponseSerialize(Worker_ComponentId component_id, void* user_data,
                                           Worker_CommandResponseHandle* handle,
                                           Schema_CommandResponse** target_out);

/* sample.ClientData */
void Sample_ClientData_CommandRequestFree(Worker_ComponentId component_id, void* user_data,
                                          Worker_CommandRequestHandle* handle);
Worker_CommandRequestHandle*
Sample_ClientData_CommandRequestCopy(Worker_ComponentId component_id, void* user_data,
                                     Worker_CommandRequestHandle* handle);
uint8_t Sample_ClientData_CommandRequestDeserialize(Worker_ComponentId component_id,
                                                    void* user_data, Schema_CommandRequest* source,
                                                    Worker_CommandRequestHandle** handle_out);
void Sample_ClientData_CommandRequestSerialize(Worker_ComponentId component_id, void* user_data,
                                               Worker_CommandRequestHandle* handle,
                                               Schema_CommandRequest** target_out);

void Sample_ClientData_CommandResponseFree(Worker_ComponentId component_id, void* user_data,
                                           Worker_CommandResponseHandle* handle);
Worker_CommandResponseHandle*
Sample_ClientData_CommandResponseCopy(Worker_ComponentId component_id, void* user_data,
                                      Worker_CommandResponseHandle* handle);
uint8_t Sample_ClientData_CommandResponseDeserialize(Worker_ComponentId component_id,
                                                     void* user_data,
                                                     Schema_CommandResponse* source,
                                                     Worker_CommandResponseHandle** handle_out);
void Sample_ClientData_CommandResponseSerialize(Worker_ComponentId component_id, void* user_data,
                                                Worker_CommandResponseHandle* handle,
                                                Schema_CommandResponse** target_out);

void Sample_ClientData_ComponentDataFree(Worker_ComponentId component_id, void* user_data,
                                         Worker_ComponentDataHandle* handle);
Worker_ComponentDataHandle* Sample_ClientData_ComponentDataCopy(Worker_ComponentId component_id,
                                                                void* user_data,
                                                                Worker_ComponentDataHandle* handle);
uint8_t Sample_ClientData_ComponentDataDeserialize(Worker_ComponentId component_id, void* user_data,
                                                   Schema_ComponentData* source,
                                                   Worker_ComponentDataHandle** handle_out);
void Sample_ClientData_ComponentDataSerialize(Worker_ComponentId component_id, void* user_data,
                                              Worker_ComponentDataHandle* handle,
                                              Schema_ComponentData** target_out);

void Sample_ClientData_ComponentUpdateFree(Worker_ComponentId component_id, void* user_data,
                                           Worker_ComponentUpdateHandle* handle);
Worker_ComponentUpdateHandle*
Sample_ClientData_ComponentUpdateCopy(Worker_ComponentId component_id, void* user_data,
                                      Worker_ComponentUpdateHandle* handle);
uint8_t Sample_ClientData_ComponentUpdateDeserialize(Worker_ComponentId component_id,
                                                     void* user_data,
                                                     Schema_ComponentUpdate* source,
                                                     Worker_ComponentUpdateHandle** handle_out);
void Sample_ClientData_ComponentUpdateSerialize(Worker_ComponentId component_id, void* user_data,
                                                Worker_ComponentUpdateHandle* handle,
                                                Schema_ComponentUpdate** target_out);
