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
#include "segway_messages.h"
#include "segwaycore.h"

int carmen_playback_nogz;
int carmen_logger_nogz;

carmen_logger_file_p infile = NULL;
double playback_starttime = 0.0;

carmen_base_odometry_message odometry;
//carmen_robot_laser_message frontlaser, rearlaser;
carmen_robot_laser_message rearlaser;
carmen_laser_laser_message frontlaser;
carmen_segway_pose_message segway;

long *message_list = NULL;
int message_list_length = 0;
int current_position = 0, offset = 0; 
int paused = 0;
int fast = 0;
int advance_frame = 0;
int rewind_frame = 0;
char *sync_tag = NULL;
int sync_tag_found = 1;

static void command_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			    void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err;
  carmen_playback_command_message command;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &command,
			   sizeof(carmen_playback_command_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  
  switch(command.cmd) {
  case CARMEN_PLAYBACK_COMMAND_PLAY:
    if(paused) {
      playback_starttime = 0.0;
      paused = 0;
      fprintf(stderr, "PLAY");
    }
    break;
  case CARMEN_PLAYBACK_COMMAND_STOP:
    if(!paused)
      paused = 1;
    break;
  case CARMEN_PLAYBACK_COMMAND_RESET:
    if(!paused)
      paused = 1;
    current_position = 0;
    playback_starttime = 0.0;
    break;
  case CARMEN_PLAYBACK_COMMAND_FORWARD:
    offset = command.arg;
    if(offset > 0 && paused)
      advance_frame = 1;
    break;
  case CARMEN_PLAYBACK_COMMAND_REWIND:
    offset = -1 * command.arg;
    if(offset < 0 && paused)
      advance_frame = 1;
    break;
  case CARMEN_PLAYBACK_COMMAND_FWD_SINGLE:
    if(!paused)
      paused = 1;
    advance_frame = 1;
    break;
  case CARMEN_PLAYBACK_COMMAND_RWD_SINGLE:
    if(!paused)
      paused = 1;
    rewind_frame = 1;
    break;
  }
}

void register_ipc_messages(void)
{
  IPC_RETURN_TYPE err;
  
  err = IPC_defineMsg(CARMEN_PLAYBACK_COMMAND_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_PLAYBACK_COMMAND_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_PLAYBACK_COMMAND_NAME);
  
  err = IPC_defineMsg(CARMEN_BASE_ODOMETRY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_ODOMETRY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_ODOMETRY_NAME);
  
  /*
  err = IPC_defineMsg(CARMEN_ROBOT_FRONTLASER_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_ROBOT_FRONTLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_ROBOT_FRONTLASER_NAME);
  */

  err = IPC_defineMsg(CARMEN_LASER_FRONTLASER_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_FRONTLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_FRONTLASER_NAME);
  
  err = IPC_defineMsg(CARMEN_ROBOT_REARLASER_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_ROBOT_REARLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_ROBOT_REARLASER_NAME);
  
  err = IPC_defineMsg(CARMEN_SEGWAY_POSE_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_SEGWAY_POSE_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_SEGWAY_POSE_NAME);
  
  err = IPC_subscribe(CARMEN_PLAYBACK_COMMAND_NAME, command_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_PLAYBACK_COMMAND_NAME);
  IPC_setMsgQueueLength(CARMEN_PLAYBACK_COMMAND_NAME, 1);
}

void index_messages(void)
{
  char line[2000], *err, message_name[50], tag[50];
  long int mark, count;
  
  message_list = (long *)calloc(1000, sizeof(long));
  carmen_test_alloc(message_list);
  message_list_length = 1000;
  count = 0;
  
  mark = carmen_playback_ftell(infile);
  err = carmen_playback_fgets(line, 2000, infile);
  if(err != NULL)
    do {
      if(line[0] != '#') {
	sscanf(line, "%s", message_name);
	if(strcmp(message_name, "PARAM") != 0) {
	  message_list[count] = mark;
	  if(sync_tag_found)
	    count++;
	  if(!sync_tag_found && sync_tag != NULL && 
	     strcmp(message_name, "SYNC") == 0) {
	    sscanf(line, "%s %s", message_name, tag);
	    if(strcmp(tag, sync_tag) == 0)
	      sync_tag_found = 1;
	  }
	  if(count == message_list_length) {
	    message_list_length *= 2;
	    message_list = (long *)realloc(message_list, 
					   message_list_length * sizeof(long));
	    carmen_test_alloc(message_list);
	  }
	}
      }
      mark = carmen_playback_ftell(infile);
      err = carmen_playback_fgets(line, 2000, infile);
    } while(err != NULL);
  message_list = (long *)realloc(message_list, count * sizeof(long));
  carmen_test_alloc(message_list);
  message_list_length = count;
  if(message_list_length == 0)
    carmen_die("Error: No messages loaded.\nEither the log file is not in the appropriate format, or the specified sync message was not found.\n");
  fprintf(stderr, "Loaded %d messages from file.\n", message_list_length);
}

void wait_for_timestamp(double ts)
{
  double current_time;
  
  if(playback_starttime == 0.0)
    playback_starttime = carmen_get_time_ms() - ts;
  current_time = carmen_get_time_ms() - playback_starttime;
  if(!fast && !paused && ts > current_time) {
    printf("napping\n");
    usleep((ts - current_time) * 1e6);
  }
}

int read_message(int message_num, int publish)
{
  char line[2000], *line_ptr, buf[1000];
  char message_name[30];
  double playback_timestamp;
  IPC_RETURN_TYPE err;
  int i;
  static int last_read = -10;

  if(message_num - last_read != 1)
    carmen_playback_fseek(infile, message_list[message_num], SEEK_SET);
  last_read = message_num;
  carmen_playback_fgets(line, 2000, infile);
  sscanf(line, "%s", message_name);
  line_ptr = line + strspn(line, " \t") + strlen(message_name);
  if(strcmp(message_name, "ODOM") == 0) {
    fprintf(stderr, "O");
    sscanf(line_ptr, "%lf %lf %lf %lf %lf %lf %lf %s %lf\n", &odometry.x,
	   &odometry.y, &odometry.theta, &odometry.tv, &odometry.rv,
	   &odometry.acceleration, &odometry.timestamp, odometry.host,
	   &playback_timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_BASE_ODOMETRY_NAME, &odometry);
    }
  } 
  if(strcmp(message_name, "FLASER") == 0) {
    fprintf(stderr, "F");
    sscanf(line_ptr, "%d", &frontlaser.num_readings);
    sprintf(buf, "%d", frontlaser.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    frontlaser.range = (float *)calloc(frontlaser.num_readings,
					sizeof(float));
    carmen_test_alloc(frontlaser.range);
    /*
    frontlaser.tooclose = (char *)calloc(frontlaser.num_readings, 1);
    carmen_test_alloc(frontlaser.tooclose);
    */
    for(i = 0; i < frontlaser.num_readings; i++) {
      sscanf(line_ptr, "%f", &frontlaser.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	line_ptr++;
    }
    /*
    sscanf(line_ptr, "%lf %lf %lf %lf %lf %lf %lf %s %lf\n", &frontlaser.x,
	   &frontlaser.y, &frontlaser.theta, &frontlaser.odom_x,
	   &frontlaser.odom_y, &frontlaser.odom_theta, &frontlaser.timestamp,
	   frontlaser.host, &playback_timestamp);
     */
    sscanf(line_ptr, "%lf %s %lf\n", &frontlaser.timestamp,
           frontlaser.host, &playback_timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      //err = IPC_publishData(CARMEN_ROBOT_FRONTLASER_NAME, &frontlaser);
      err = IPC_publishData(CARMEN_LASER_FRONTLASER_NAME, &frontlaser);
    }
    free(frontlaser.range);
    /* free(frontlaser.tooclose);*/
    return 1;
  }
  if(strcmp(message_name, "RLASER") == 0) {
    fprintf(stderr, "R");
    sscanf(line_ptr, "%d", &rearlaser.num_readings);
    sprintf(buf, "%d", rearlaser.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    rearlaser.range = (float *)calloc(rearlaser.num_readings,
				       sizeof(float));
    carmen_test_alloc(rearlaser.range);
    rearlaser.tooclose = (char *)calloc(rearlaser.num_readings, 1);
    carmen_test_alloc(rearlaser.tooclose);
    for(i = 0; i < rearlaser.num_readings; i++) {
      sscanf(line_ptr, "%f", &rearlaser.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	line_ptr++;
    }
    sscanf(line_ptr, "%lf %lf %lf %lf %lf %lf %lf %s %lf\n", &rearlaser.x,
	   &rearlaser.y, &rearlaser.theta, &rearlaser.odom_x,
	   &rearlaser.odom_y, &rearlaser.odom_theta, &rearlaser.timestamp,
	   rearlaser.host, &playback_timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_ROBOT_REARLASER_NAME, &rearlaser);
    }
    free(rearlaser.range);
    free(rearlaser.tooclose);
  }
  if (strcmp(message_name, "SEGWAY") == 0) {
    fprintf(stderr, "S");
    sscanf(line_ptr, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %s %lf\n",
	   &segway.pitch, &segway.pitch_rate, &segway.roll,
	   &segway.roll_rate, &segway.lw_velocity, &segway.rw_velocity,
	   &segway.x, &segway.y, &segway.theta, &segway.timestamp,
	   segway.host, &playback_timestamp);
    if (publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_SEGWAY_POSE_NAME, &segway);
    }
  }
  return 0;
}

void main_playback_loop(void)
{
  int laser;
  
  while(1) {

    if(offset != 0) {
      playback_starttime = 0.0;
      current_position += offset;
      if(current_position < 0)
	current_position = 0;
      if (current_position >= message_list_length)
	current_position = message_list_length-1;
      offset = 0;
    }
    
    if(!paused && current_position < message_list_length) {
      read_message(current_position, 1);
      current_position++;
    }
    else if(paused && advance_frame) {
      laser = 0;
      while(current_position < message_list_length && !laser) {
	laser = read_message(current_position, 1);
	current_position++;
      } 
      advance_frame = 0;
    }
    else if(paused && rewind_frame) {
      laser = 0;
      while(current_position > 0 && !laser) {
	current_position--;
	laser = read_message(current_position, 0);
      } 
      laser = 0;
      while(current_position > 0 && !laser) {
	current_position--;
	laser = read_message(current_position, 0);
      }
      read_message(current_position, 1);
      current_position++;
      rewind_frame = 0;
    }
    if (fast) {
      sleep_ipc(0.001);
    } else {
      sleep_ipc(0.1);
    }
  }
}

void usage(char *fmt, ...) 
{
  va_list args;
  
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  
  fprintf(stderr, "Usage: segwayplayback filename <args>\n");
  fprintf(stderr, "\t-fast         - ignore timestamps.\n");
  fprintf(stderr, "\t-paused       - start paused.\n");
  fprintf(stderr, "\t-syncto tag   - start from the tag tag\n");
  exit(-1);
}

void read_parameters(int argc, char **argv)
{
  int index;

  if(argc < 2)
    usage("Needs at least one argument.\n");

  for (index = 0; index < argc; index++) 
    {
      if (strncmp(argv[index], "-h", 2) == 0 || 
	  strncmp(argv[index], "--help", 6) == 0)
	usage(NULL);
      if (strncmp(argv[index], "-paused", 7) == 0)
	paused = 1;
      if (strncmp(argv[index], "-fast", 5) == 0) {
	fprintf(stderr, "Entering fast mode\n");
	fast = 1;
      }
      if (strncmp(argv[index], "-syncto", 7) == 0)
	{
	  if (index == argc-1)
	    usage(NULL);
	  index++;
	  sync_tag = argv[index];
	  sync_tag_found = 0;
	  paused = 1;	  
	}
    }
}

void shutdown_module(int sig)
{
  if(sig == SIGINT) {
    fprintf(stderr, "Disconnecting.\n");
    exit(1);
  }
}

int main(int argc, char **argv)
{
  char *filename;
  
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  
  register_ipc_messages();
  read_parameters(argc, argv);
  signal(SIGINT, shutdown_module);
  
  filename = argv[1];
  carmen_playback_nogz = 0;
  if (strcmp(filename + strlen(filename) - 3, ".gz"))
    carmen_playback_nogz = 1;
#ifdef NO_ZLIB
  else
    carmen_die("Error: cannot read gzipped log files: %s.", filename);
#endif

  infile = carmen_playback_fopen(filename, "r");
  if(infile == NULL)
    carmen_die("Error: could not open file %s for reading.\n", filename);
  index_messages();
  
  main_playback_loop();
  return 0;
}