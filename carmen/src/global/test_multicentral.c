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
  int i;

  /* connect to all IPC servers */
  carmen_multicentral_allow_zero_centrals(1);
  centrallist = carmen_multicentral_initialize(argc, argv, 
					       test_ipc_exit_handler);

  carmen_multicentral_start_central_check(centrallist);

  /* subscribe to messages from each central */
  carmen_multicentral_subscribe_messages(centrallist, 
					 test_subscribe_messages);

  do {
    /* handle IPC messages for each central */
    for(i = 0; i < centrallist->num_centrals; i++) 
      if(centrallist->central[i].connected) {
        IPC_setContext(centrallist->central[i].context);
        carmen_ipc_sleep(0.05);
      }
      else
	usleep(50000);

    /* attempt to reconnect any missing centrals */
    carmen_multicentral_reconnect_centrals(centrallist, NULL,
					   test_subscribe_messages);
  } while(1);
  return 0;
}
