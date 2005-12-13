#ifndef CARMEN_PIDCONTROL_H
#define CARMEN_PIDCONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#define        MAX_PROCESSES          100

typedef struct {
  char *module_name, *group_name;
  char command_line[1000];
  int pid, pipefd[2];
  int requested_state, state;
  double start_time;
  int watch_heartbeats;
  double last_heartbeat;
} process_info_t, *process_info_p;

#ifdef __cplusplus
}
#endif

#endif
