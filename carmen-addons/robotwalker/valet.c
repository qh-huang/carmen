
#include <carmen/carmen.h>
#include <carmen/map_io.h>


static carmen_map_placelist_t parking_spots;


static void valet_park() {


}

static void valet_return() {


}

static void valet_park_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			       void *clientData __attribute__ ((unused))) {

  FORMATTER_PTR formatter;

  formatter = IPC_msgInstanceFormatter(msgRef);
  IPC_freeByteArray(callData);

  valet_park();
}

static void valet_return_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
				 void *clientData __attribute__ ((unused))) {

  FORMATTER_PTR formatter;

  formatter = IPC_msgInstanceFormatter(msgRef);
  IPC_freeByteArray(callData);

  valet_return();
}

static void ipc_init() {

  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_VALET_PARK_MSG_NAME, IPC_VARIABLE_LENGTH, CARMEN_VALET_PARK_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CAMRNE_VALET_PARK_MSG_NAME);

  err = IPC_defineMsg(CARMEN_VALET_RETURN_MSG_NAME, IPC_VARIABLE_LENGTH, CARMEN_VALET_RETURN_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CAMRNE_VALET_RETURN_MSG_NAME);

  err = IPC_subscribe(CARMEN_VALET_PARK_MSG_NAME, valet_park_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_VALET_PARK_MSG_NAME);
  IPC_setMsgQueueLength(CARMEN_VALET_PARK_MSG_NAME, 1);

  err = IPC_subscribe(CARMEN_VALET_RETURN_MSG_NAME, valet_return_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_VALET_RETURN_MSG_NAME);
  IPC_setMsgQueueLength(CARMEN_VALET_RETURN_MSG_NAME, 1);
}

void valet_init() {

  carmen_map_get_placelist(&parking_spots);
}

int main(int argc, char **argv) {

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  ipc_init();
  valet_init();
}
