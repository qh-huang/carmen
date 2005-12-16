#include <carmen/carmen.h>
#include <carmen/multicentral.h>

void test_subscribe_messages(void)
{

}

void test_ipc_exit_handler(void)
{
  fprintf(stderr, "Central died.\n");
}

int main(int argc, char **argv)
{
  carmen_centrallist_p centrallist;

  /* set this if it is OK for the program to run without connections
     to any centrals */
  carmen_multicentral_allow_zero_centrals(1);

  /* connect to all IPC servers */
  centrallist = carmen_multicentral_initialize(argc, argv, 
					       test_ipc_exit_handler);

  /* start thread that monitors connections to centrals */
  carmen_multicentral_start_central_check(centrallist);

  /* subscribe to messages from each central */
  carmen_multicentral_subscribe_messages(centrallist, 
					 test_subscribe_messages);

  do {
    /* handle IPC messages across all centrals */
    carmen_multicentral_ipc_sleep(centrallist, 0.1);

    /* attempt to reconnect any missing centrals */
    carmen_multicentral_reconnect_centrals(centrallist, NULL,
					   test_subscribe_messages);
  } while(1);
  return 0;
}
