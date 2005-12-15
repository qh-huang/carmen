#ifndef CARMEN_MULTICENTRAL_H
#define CARMEN_MULTICENTRAL_H

#include <carmen/carmen.h>

typedef struct {
  int connected, ready_for_reconnect;
  char host[256];
  IPC_CONTEXT_PTR context;
} carmen_central_t, *carmen_central_p;

typedef struct {
  int num_centrals;
  carmen_central_p central;
} carmen_centrallist_t, *carmen_centrallist_p;

#ifdef __cplusplus
extern "C" {
#endif

void carmen_multicentral_allow_zero_centrals(int allow);

carmen_centrallist_p carmen_multicentral_initialize(int argc, char **argv,
						    void (*exit_handler)(void));

void carmen_multicentral_get_params(carmen_centrallist_p centrallist, 
				    int argc, char **argv,
				    void (*param_func)(int, char **));

void carmen_multicentral_register_messages(carmen_centrallist_p centrallist,
					   void (*register_func)(void));

void carmen_multicentral_subscribe_messages(carmen_centrallist_p centrallist,
					    void (*subscribe_func)(void));

void carmen_multicentral_reconnect_centrals(carmen_centrallist_p centrallist,
					    void (*register_func)(void),
					    void (*subscribe_func)(void));

void carmen_multicentral_start_central_check(carmen_centrallist_p centrallist);

#ifdef __cplusplus
}
#endif

#endif
