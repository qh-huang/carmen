#ifndef CARMEN_PROCCONTROL_IPC_H
#define CARMEN_PROCCONTROL_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <carmen/carmen.h>
#include "proccontrol.h"

void proccontrol_register_ipc_messages(void);

void proccontrol_publish_output(int pid, char *output);

void proccontrol_publish_pidtable(int num_processes, process_info_p process);

#ifdef __cplusplus
}
#endif

#endif
