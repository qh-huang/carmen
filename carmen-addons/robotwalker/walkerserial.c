#include <carmen/carmen.h>
#include <carmen/serial.h>
#include "walkerserial_interface.h"


static int fd;


static void publish_button_msg(int button) {

  static carmen_walkerserial_button_msg button_msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  printf("publish_button_msg(%d)\n", button);

  if (first) {
    strcpy(button_msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  button_msg.timestamp = carmen_get_time_ms();
  button_msg.button = button;

  err = IPC_publishData(CARMEN_WALKERSERIAL_BUTTON_MSG_NAME,
			&button_msg);
  carmen_test_ipc_exit(err, "Could not publish",
		       CARMEN_WALKERSERIAL_BUTTON_MSG_NAME);  
}

int open_serial_port(char *dev) {

  int fd;
  struct termios newtio;

  fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
  //fd = open(dev, O_WRONLY | O_NOCTTY | O_NDELAY);
  if(fd == -1) {
    fprintf(stderr, "Couldn't open serial port %s\n", dev);
    return -1;
  }
  
  memset(&newtio, 0, sizeof(newtio));
  newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR | ICRNL;
  newtio.c_oflag = 0;
  newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  //= ICANON;
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &newtio);

  fcntl(fd, F_SETFL, FNDELAY);

  return fd;
}

static void init_ipc() {

  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_WALKERSERIAL_BUTTON_MSG_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_WALKERSERIAL_BUTTON_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define",
		       CARMEN_WALKERSERIAL_BUTTON_MSG_NAME);
}

int main(int argc, char *argv[]) {

  int i, j;
  unsigned char buf[2];

  if (argc < 2)
    carmen_die("usage: walkerserial <serial device>\n");

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  init_ipc();

  for (i = 0; i < 10; i++)
    if ((fd = open_serial_port(argv[1])) >= 0)
      break;
  if (i == 10)
    carmen_die("Error: could not open serial port %s.\n", argv[1]);

  while (1) {
    sleep_ipc(0.01);
    usleep(10000);
    i = carmen_serial_numChars(fd);
    if (i > 0) {
      i = carmen_serial_readn(fd, buf, i);
      if (i <= 0)
	continue;
      for (j = 0; j < i; j++) {
	if (buf[j] >= 0x40)
	  continue;
	if (buf[j] & 0x1)
	  publish_button_msg(1);
	if (buf[j] & 0x2)
	  publish_button_msg(2);
	if (buf[j] & 0x4)
	  publish_button_msg(3);
	if (buf[j] & 0x8)
	  publish_button_msg(4);
	if (buf[j] & 0x10)
	  publish_button_msg(5);
	if (buf[j] & 0x20)
	  publish_button_msg(6);
      }
    }
  }
}
