/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
 *
 * CARMEN is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option)
 * any later version.
 *
 * CARMEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General 
 * Public License along with CARMEN; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/
#include <carmen/carmen.h>
#include "cerebellum_com.h"
#include "cereb_commands.h"
//#define DEBUG 1

#define CEREB_BAUDRATE 115200
#define PIC_WRITE_DELAY      10 // serial xfer delay, in us
                                // keeps pic from losing
                                // serial data
#define PIC_READ_DELAY 500     // xfer delay, in us
                                // gives pic a chance to respond

static int dev_fd;

int
cereb_send_string(char * ptr, int length)
{

  int i;
  
  for(i = 0; i < length; ++i)
    {
      carmen_serial_writen(dev_fd, ptr+i, 1);
      // keeps the PIC from getting overwhelmed
      usleep(PIC_WRITE_DELAY);

#ifdef DEBUG
      printf("%x ",ptr[i]);
#endif
    }

  return 0;
}

int
cereb_read_string(char * ptr, int length)
{
  int x;
  int i;

  x = 0;

  for(i = 0; i < length; ++i)
    {
      // keeps the PIC from getting overwhelmed
      usleep(PIC_WRITE_DELAY);
      x = carmen_serial_readn(dev_fd, ptr+i, 1);

      if(x == -1)
	return -1;

#ifdef DEBUG
      printf("%x ",ptr[i]);
#endif
    }

#ifdef DEBUG
  printf("\n");
  printf("return val is: %x\n", x);
#endif

  return x;
}

// puts 32-bit ints into buffer in little-endian order for PIC to accept
void stuff_buffer(char *ptr, int * index, int data)
{
  int i;

  for(i = 0; i < 25;)
    {
      ptr[(*index)++] = (data >> i)& 0xFF;
      i += 8;
    }
}

int unpack_buffer(unsigned char *ptr, int * index)
{
  //  int i;
  unsigned char0,char1,char2,char3;
  int data = 0;

  char0 = ptr[(*index)++];
  char1 = ptr[(*index)++];
  char2 = ptr[(*index)++]; 
  char3 = ptr[(*index)++];

  data |=  ((int)char0)        & 0x000000FF;
  data |= (((int)char1) << 8)  & 0x0000FF00;
  data |= (((int)char2) << 16) & 0x00FF0000;
  data |= (((int)char3) << 24) & 0xFF000000;

  /*
  for(i = 0; i < 25;)
    {
      data |= (((int)(ptr[(*index)++])) & 0xFF) << i;

      i += 8;
    }
  */
  /*
  data = (int)ptr[(*index)];
  *index += 4;
  */

  return data;
}

// skim through a buffer and validate the checksum.
// index is the value of the checksum(thus the data is
// from index 0 ... index - 1)
int verify_checksum(char *ptr, int index)
{
  int i;
  char checksum = 0;

  for(i = 0; i < index; ++i)
    {
      checksum ^= ptr[i];
    }

  if(checksum == ptr[index])
    return 0;
  else
    return -1;
}

// calculates checksum
// command ^ data0 ^ data1 ^ ... 
// i.e. everything sent is xor'd and that checksum is added
// only used where data is being sent, not commands
void checksum_data(char *ptr, int * index)
{
  int i;
  unsigned char chksum = 0;

  for(i = 0; i < *index; ++i)
    {
      chksum ^= ptr[i];
    }

  ptr[(*index)++] = chksum;
}

// returns the status of the Cerebellum acknowledgement.
int check_ack()
{
  char buf[1];

  //check for ack
  cereb_read_string(buf, 1);   

  if(buf[0] == ACKNOWLEDGE)
    {
#ifdef DEBUG
      printf("ACK\n");
#endif
      return 0;
    }
  else if(buf[0] == NACKNOWLEDGE)
    {
#ifdef DEBUG
      printf("NACK\n");
#endif
      return -1;
    }
  else
    {
#ifdef DEBUG
      printf("UNKNOWN: %x",buf[0]);
#endif
      return -2;
    }
}

static int 
cereb_init(void) 
{
  unsigned char buf[3];

  buf[0] = INIT1;
  buf[1] = INIT2;
  buf[2] = INIT3;

  cereb_send_string(buf, 3);
  return check_ack();
}

int 
cereb_send_command(char command) 
{
  unsigned char buf[1];

  buf[0] = command;

  cereb_send_string(buf, 1);
  return check_ack();
}

// sends left and right wheel velocities to mach5
static int cereb_send_2int_command(char command, int left, int right)
{
  unsigned char buf[20];
  int index = 0;

  buf[index++] = command;
  stuff_buffer(buf, &index, -left);
  stuff_buffer(buf, &index, right);
  checksum_data(buf, &index);
  cereb_send_string(buf, index);

  return check_ack();
}

static int cereb_init_serial(char * dev)
{
  if (carmen_serial_connect(&dev_fd, dev) < 0)
    return -1;

  //sets baud rate to CEREB_BAUDRATE
  carmen_serial_configure(dev_fd, CEREB_BAUDRATE, "N");

  return 0;
}  

int 
carmen_cerebellum_connect_robot(char *dev)
{
  if(cereb_init_serial(dev) < 0)
    return -1;

  return cereb_init();
}

int carmen_cerebellum_ac(int acc)
{
  return cereb_send_2int_command(SET_ACCELERATIONS,acc,acc);
}

int 
carmen_cerebellum_set_velocity(int command_vl, int command_vr)
{
  return cereb_send_2int_command(SET_VELOCITIES,command_vl, command_vr);
}

int
carmen_cerebellum_get_voltage(int *batt_voltage)
{
  unsigned char buf[20];
  int index = 0;

  if(cereb_send_command(GET_BATT_VOLTAGE) < 0)
    return -1;

  usleep(PIC_READ_DELAY);

  if(cereb_read_string(buf, 5) < 0)
    return -1;

  if(verify_checksum(buf, 4) < 0)
    return -1;

  *batt_voltage = unpack_buffer(buf, &index);

  return 0;
}

int 
carmen_cerebellum_get_state(int *left_tics, int *right_tics,
			    int *left_vel, int *right_vel)
{
  unsigned char buf[20];
  int index = 0;

  if(cereb_send_command(GET_ODOMETRY) < 0)
    return -1;

  // wait a bit for response to come
  usleep(PIC_READ_DELAY);

  // read 4 int32's, and 1 checksum
  if(cereb_read_string(buf, 17) < 0)
    return -1;

  if(verify_checksum(buf, 16) < 0)
    return -1;

  *left_tics = unpack_buffer(buf, &index);
  *right_tics = unpack_buffer(buf, &index);
  *left_vel = unpack_buffer(buf, &index);
  *right_vel = unpack_buffer(buf, &index);
  
  return 0;
}

int 
carmen_cerebellum_limp(void)
{
  return cereb_send_command(DISABLE_VELOCITY_CONTROL);
}

int
carmen_cerebellum_engage(void)
{
  return cereb_send_command(ENABLE_VELOCITY_CONTROL);
}

int 
carmen_cerebellum_disconnect_robot(void)
{
  return cereb_send_command(DEINIT);
}
