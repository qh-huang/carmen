#include <carmen/carmen.h>


void x_ipcRegisterExitProc(void (*proc)(void));
void reconnect(void);

char *module_name;

void handle_laser(carmen_robot_laser_message *laser)
{
  carmen_warn(".");
}

IPC_RETURN_TYPE connect_ipc(void)
{
  IPC_RETURN_TYPE err;

  IPC_setVerbosity(IPC_Silent);

  err = IPC_connect(module_name);
  if (err != IPC_OK)
    return err;

  x_ipcRegisterExitProc(reconnect);
  
  carmen_robot_subscribe_frontlaser_message
    (NULL, (carmen_handler_t)handle_laser, CARMEN_SUBSCRIBE_LATEST);

  return err;
}

void reconnect(void)
{
  IPC_RETURN_TYPE err;

  do {
    carmen_warn("IPC died. Reconnecting...\n");
    if (IPC_isConnected())
      IPC_disconnect();
    err = connect_ipc();
    if (err == IPC_OK)
      carmen_warn("Reconnected...\n");
  } while (err == IPC_Error);
}

int main (int argc, char *argv[])
{
  module_name = argv[0];

  connect_ipc();

  x_ipcRegisterExitProc(reconnect);
  
  carmen_robot_subscribe_frontlaser_message
    (NULL, (carmen_handler_t)handle_laser, CARMEN_SUBSCRIBE_LATEST);
 
  IPC_dispatch();
}
