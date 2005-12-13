#ifndef CARMEN_PROCCONTROL_MESSAGES_H
#define CARMEN_PROCCONTROL_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double timestamp;
  char *host;
  char *group_name;
  int requested_state;
} carmen_proccontrol_groupset_message;

#define     CARMEN_PROCCONTROL_GROUPSET_NAME    "carmen_proccontrol_groupset"
#define     CARMEN_PROCCONTROL_GROUPSET_FMT     "{double,string,string,int}"

typedef struct {
  double timestamp;
  char *host;
  char *module_name;
  int requested_state;
} carmen_proccontrol_moduleset_message;

#define     CARMEN_PROCCONTROL_MODULESET_NAME   "carmen_proccontrol_moduleset"
#define     CARMEN_PROCCONTROL_MODULESET_FMT    "{double,string,string,int}"

typedef struct {
  char *group_name, *module_name;
  int active, requested_state, pid;
} carmen_proccontrol_process_t, *carmen_proccontrol_process_p;

typedef struct {
  double timestamp;
  char *host;
  int num_processes;
  carmen_proccontrol_process_p process;
} carmen_proccontrol_pidtable_message;

#define     CARMEN_PROCCONTROL_PIDTABLE_NAME     "carmen_proccontrol_pidtable"
#define     CARMEN_PROCCONTROL_PIDTABLE_FMT      "{double,string,int,<{string,string,int,int,int}:3>}"

typedef struct {
  double timestamp;
  char *host;
  int pid;
  char *output;
} carmen_proccontrol_output_message;

#define     CARMEN_PROCCONTROL_OUTPUT_NAME       "carmen_proccontrol_output"
#define     CARMEN_PROCCONTROL_OUTPUT_FMT        "{double,string,int,string}"

#ifdef __cplusplus
}
#endif

#endif
