#define   MAX_STATUS_TIME  3.0

extern char   host_name[MAX_NAME_LENGTH];

void      carmen_initialize_ipc(char *module_name);
void      ipc_initialize_messages( void );
void      ipc_publish_position( void );
