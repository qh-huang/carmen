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

int carmen_playback_nogz;
int carmen_logger_nogz;

const int max_line_length = 10000; 
int print_timestamp = 0;


carmen_logger_file_p infile = NULL;
double playback_starttime = 0.0;

carmen_simulator_truepos_message truepos;
carmen_base_odometry_message odometry;
carmen_robot_laser_message frontlaser, rearlaser;
carmen_laser_laser_message laser3, laser4;
// *** REI - START *** //
carmen_laser_remission_message frontlaserremission, rearlaserremission, laser3remission, laser4remission;
// *** REI - END *** //
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
      if (!print_timestamp)
	fprintf(stderr, " PLAY ");
      else
	fprintf(stderr, "PLAY\n");
    }
    break;
  case CARMEN_PLAYBACK_COMMAND_STOP:
    if(!paused) {
      paused = 1;
      if (!print_timestamp)
	fprintf(stderr, " STOP ");
      else
	fprintf(stderr, "STOP  ");
    }
    break;
  case CARMEN_PLAYBACK_COMMAND_RESET:
    if(!paused)
      paused = 1;
    current_position = 0;
    playback_starttime = 0.0;
    if (!print_timestamp)
      fprintf(stderr, "\nRESET ");
    else
      fprintf(stderr, "\nRESET\n");
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
  
  err = IPC_defineMsg(CARMEN_ROBOT_FRONTLASER_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_ROBOT_FRONTLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_ROBOT_FRONTLASER_NAME);

  err = IPC_defineMsg(CARMEN_LASER_LASER3_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_LASER3_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_LASER3_NAME);
  
  err = IPC_defineMsg(CARMEN_LASER_LASER4_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_LASER4_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_LASER4_NAME);

// *** REI - START *** //
  err = IPC_defineMsg(CARMEN_LASER_FRONTLASER_REMISSION_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_FRONTLASER_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_FRONTLASER_REMISSION_NAME);

  err = IPC_defineMsg(CARMEN_LASER_REARLASER_REMISSION_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_REARLASER_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_REARLASER_REMISSION_NAME);

  err = IPC_defineMsg(CARMEN_LASER_LASER3_REMISSION_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_LASER3_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_LASER3_REMISSION_NAME);

  err = IPC_defineMsg(CARMEN_LASER_LASER4_REMISSION_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_LASER4_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_LASER4_REMISSION_NAME);


// *** REI - END *** //

  err = IPC_defineMsg(CARMEN_ROBOT_REARLASER_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_ROBOT_REARLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_ROBOT_REARLASER_NAME);
  
  err = IPC_subscribe(CARMEN_PLAYBACK_COMMAND_NAME, command_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_PLAYBACK_COMMAND_NAME);
  IPC_setMsgQueueLength(CARMEN_PLAYBACK_COMMAND_NAME, 1);
}

void index_messages(void)
{
  int count = 0, buffer_pos;
  unsigned int buffer_length, mark, n, offset = 0;
  char message_name[50], tag[50], buffer[10000];

  message_list = (long *)calloc(1000, sizeof(long));
  carmen_test_alloc(message_list);
  message_list_length = 1000;
  count = 0;

  buffer_pos = 0;
  buffer_length = carmen_playback_fread(buffer, 1, 10000, infile);
  do {
    if(buffer_length > 0) {
      mark = buffer_pos;
      while(mark < buffer_length && buffer[mark] != '\n')
	mark++;
      if(mark == buffer_length) {
	memmove(buffer, buffer + buffer_pos, buffer_length - buffer_pos);
	buffer_length -= buffer_pos;
	offset += buffer_pos;
	buffer_pos = 0;
	n = carmen_playback_fread(buffer + buffer_length, 
				  1, 10000 - buffer_length - 1, infile);
	buffer_length += n;
      }
      else {
	if(buffer[buffer_pos] != '#') {
	  sscanf(buffer + buffer_pos, "%s", message_name);
	  if(strcmp(message_name, "PARAM") != 0) {
	    message_list[count] = offset + buffer_pos;
	    if(sync_tag_found)
	      count++;
	    if(!sync_tag_found && sync_tag != NULL && 
	       strcmp(message_name, "SYNC") == 0) {
	      sscanf(buffer + buffer_pos, "%s %s", message_name, tag);
	      if(strcmp(tag, sync_tag) == 0)
		sync_tag_found = 1;
	    }
	    if(count == message_list_length) {
	      message_list_length *= 2;
	      message_list = (long *)realloc(message_list, 
					     message_list_length * 
					     sizeof(long));
	      carmen_test_alloc(message_list);
	    }
	  }
	}
	buffer_pos = mark + 1;
      }
    }
  } while(buffer_length > 0);
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
    playback_starttime = carmen_get_time() - ts;
  current_time = carmen_get_time() - playback_starttime;
  if(!fast && !paused && ts > current_time)
    usleep((ts - current_time) * 1e6);
}

int read_message(int message_num, int publish)
{
  char line[max_line_length], *line_ptr, buf[1000];
  char message_name[30];
  double playback_timestamp;
  IPC_RETURN_TYPE err;
  int i;
  static int last_read = -10;

  if(message_num - last_read != 1)
    carmen_playback_fseek(infile, message_list[message_num], SEEK_SET);
  last_read = message_num;
  carmen_playback_fgets(line, max_line_length, infile);
  sscanf(line, "%s", message_name);
  line_ptr = line + strspn(line, " \t") + strlen(message_name);
  if(strcmp(message_name, "ODOM") == 0) {
    sscanf(line_ptr, "%lf %lf %lf %lf %lf %lf %lf %s %lf\n", &odometry.x,
	   &odometry.y, &odometry.theta, &odometry.tv, &odometry.rv,
	   &odometry.acceleration, &odometry.timestamp, odometry.host,
	   &playback_timestamp);

    if (!print_timestamp)
      fprintf(stderr, "O");
    else
      fprintf(stderr, "%lf  (O)\n", odometry.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_BASE_ODOMETRY_NAME, &odometry);
    }
  } 
  if(strcmp(message_name, "TRUEPOS") == 0) {
    sscanf(line_ptr, "%lf %lf %lf %lf %lf %lf %lf %s %lf\n", 
	   &truepos.truepose.x, &truepos.truepose.y, 
	   &truepos.truepose.theta,
	   &truepos.odometrypose.x, &truepos.odometrypose.y, 
	   &truepos.odometrypose.theta,
	   &truepos.timestamp, truepos.host,
	   &playback_timestamp);

    if (!print_timestamp)
      fprintf(stderr, "T");
    else
      fprintf(stderr, "%lf  (T)\n", truepos.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_SIMULATOR_TRUEPOS_NAME, &truepos);
    }
  } 
  else if(strcmp(message_name, "FLASER") == 0) {
    sscanf(line_ptr, "%d", &frontlaser.num_readings);
    sprintf(buf, "%d", frontlaser.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    frontlaser.range = (float *)calloc(frontlaser.num_readings,
					sizeof(float));
    carmen_test_alloc(frontlaser.range);
    frontlaser.tooclose = (char *)calloc(frontlaser.num_readings, 1);
    carmen_test_alloc(frontlaser.tooclose);
    for(i = 0; i < frontlaser.num_readings; i++) {
      sscanf(line_ptr, "%f", &frontlaser.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	line_ptr++;
    }
    sscanf(line_ptr, "%lf %lf %lf %lf %lf %lf %lf %s %lf\n", &frontlaser.x,
	   &frontlaser.y, &frontlaser.theta, &frontlaser.odom_x,
	   &frontlaser.odom_y, &frontlaser.odom_theta, &frontlaser.timestamp,
	   frontlaser.host, &playback_timestamp);
    if (!print_timestamp)
      fprintf(stderr, "F");
    else
      fprintf(stderr, "%lf  (F)\n", frontlaser.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_ROBOT_FRONTLASER_NAME, &frontlaser);
    }
    free(frontlaser.range);
    free(frontlaser.tooclose);
    return 1;
  }
  else if(strcmp(message_name, "RLASER") == 0) {
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
    if (!print_timestamp)
      fprintf(stderr, "R");
    else
      fprintf(stderr, "%lf  (R)\n", rearlaser.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_ROBOT_REARLASER_NAME, &rearlaser);
    }
    free(rearlaser.range);
    free(rearlaser.tooclose);
  }
  else if(strcmp(message_name, "LASER3") == 0) {
    sscanf(line_ptr, "%d", &laser3.num_readings);
    sprintf(buf, "%d", laser3.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    laser3.range = (float *)calloc(laser3.num_readings,
				       sizeof(float));
    carmen_test_alloc(laser3.range);
    for(i = 0; i < laser3.num_readings; i++) {
      sscanf(line_ptr, "%f", &laser3.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	line_ptr++;
    }
    sscanf(line_ptr, "%lf %s %lf\n", &laser3.timestamp,
	   laser3.host, &playback_timestamp);
    if (!print_timestamp)
      fprintf(stderr, "3");
    else
      fprintf(stderr, "%lf  (3)\n", laser3.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_LASER_LASER3_NAME, &laser3);
    }
    free(laser3.range);
  }
  else if(strcmp(message_name, "LASER4") == 0) {
    sscanf(line_ptr, "%d", &laser4.num_readings);
    sprintf(buf, "%d", laser4.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    laser4.range = (float *)calloc(laser4.num_readings,
				       sizeof(float));
    carmen_test_alloc(laser4.range);
    for(i = 0; i < laser4.num_readings; i++) {
      sscanf(line_ptr, "%f", &laser4.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	line_ptr++;
    }
    sscanf(line_ptr, "%lf %s %lf\n", &laser4.timestamp,
	   laser4.host, &playback_timestamp);
    if (!print_timestamp)
      fprintf(stderr, "4");
    else
      fprintf(stderr, "%lf  (4)\n", laser4.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_LASER_LASER4_NAME, &laser4);
    }
    free(laser4.range);
  }
// *** REI - START *** //
  else if(strcmp(message_name, "REMISSIONFLASER") == 0) {
    sscanf(line_ptr, "%d", &frontlaserremission.num_readings);
    sprintf(buf, "%d", frontlaserremission.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    frontlaserremission.range = (float *)calloc(frontlaserremission.num_readings,
					sizeof(float));
    carmen_test_alloc(frontlaserremission.range);
    frontlaserremission.remission = (float *)calloc(frontlaserremission.num_readings,
						   sizeof(float));
    carmen_test_alloc(frontlaserremission.remission);

    for(i = 0; i < frontlaserremission.num_readings; i++) {
      sscanf(line_ptr, "%f", &frontlaserremission.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	    line_ptr++;

      sscanf(line_ptr, "%f", &frontlaserremission.remission[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	    line_ptr++;
    }
    sscanf(line_ptr, "%lf %s %lf\n",  &frontlaserremission.timestamp,
	   frontlaserremission.host, &playback_timestamp);
    if (!print_timestamp)
      fprintf(stderr, "f");
    else
      fprintf(stderr, "%lf  (f)\n", frontlaserremission.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_LASER_FRONTLASER_REMISSION_NAME, &frontlaserremission);
    }
    free(frontlaserremission.range);
    free(frontlaserremission.remission);
    return 1;
  }
  else if(strcmp(message_name, "REMISSIONRLASER") == 0) {
    sscanf(line_ptr, "%d", &rearlaserremission.num_readings);
    sprintf(buf, "%d", rearlaserremission.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    rearlaserremission.range = (float *)calloc(rearlaserremission.num_readings,
					sizeof(float));
    carmen_test_alloc(rearlaserremission.range);
    rearlaserremission.remission = (float *)calloc(rearlaserremission.num_readings,
						   sizeof(float));
    carmen_test_alloc(rearlaserremission.remission);

    for(i = 0; i < rearlaserremission.num_readings; i++) {
      sscanf(line_ptr, "%f", &rearlaserremission.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	    line_ptr++;

      sscanf(line_ptr, "%f", &rearlaserremission.remission[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	    line_ptr++;
    }
    sscanf(line_ptr, "%lf %s %lf\n",  &rearlaserremission.timestamp,
	   rearlaserremission.host, &playback_timestamp);
    if (!print_timestamp)
      fprintf(stderr, "r");
    else
      fprintf(stderr, "%lf  (r)\n", rearlaserremission.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_LASER_REARLASER_REMISSION_NAME, &rearlaserremission);
    }
    free(rearlaserremission.range);
    free(rearlaserremission.remission);
    return 1;
  }
  else if(strcmp(message_name, "REMISSIONLASER3") == 0) {
    sscanf(line_ptr, "%d", &laser3remission.num_readings);
    sprintf(buf, "%d", laser3remission.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    laser3remission.range = (float *)calloc(laser3remission.num_readings,
					sizeof(float));
    carmen_test_alloc(laser3remission.range);
    laser3remission.remission = (float *)calloc(laser3remission.num_readings,
						   sizeof(float));
    carmen_test_alloc(laser3remission.remission);

    for(i = 0; i < laser3remission.num_readings; i++) {
      sscanf(line_ptr, "%f", &laser3remission.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	    line_ptr++;

      sscanf(line_ptr, "%f", &laser3remission.remission[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	    line_ptr++;
    }
    sscanf(line_ptr, "%lf %s %lf\n",  &laser3remission.timestamp,
	   laser3remission.host, &playback_timestamp);
    if (!print_timestamp)
      fprintf(stderr, "3");
    else
      fprintf(stderr, "%lf  (3*)\n", laser3remission.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_LASER_LASER3_REMISSION_NAME, &laser3remission);
    }
    free(laser3remission.range);
    free(laser3remission.remission);
    return 1;
  }
  else if(strcmp(message_name, "REMISSIONLASER4") == 0) {
    sscanf(line_ptr, "%d", &laser4remission.num_readings);
    sprintf(buf, "%d", laser4remission.num_readings);
    line_ptr += strspn(line_ptr, " \t") + strlen(buf);
    laser4remission.range = (float *)calloc(laser4remission.num_readings,
					sizeof(float));
    carmen_test_alloc(laser4remission.range);
    laser4remission.remission = (float *)calloc(laser4remission.num_readings,
						   sizeof(float));
    carmen_test_alloc(laser4remission.remission);

    for(i = 0; i < laser4remission.num_readings; i++) {
      sscanf(line_ptr, "%f", &laser4remission.range[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	    line_ptr++;

      sscanf(line_ptr, "%f", &laser4remission.remission[i]);
      line_ptr += strspn(line_ptr, " \t");
      while (*line_ptr != ' ')
	    line_ptr++;
    }
    sscanf(line_ptr, "%lf %s %lf\n",  &laser4remission.timestamp,
	   laser4remission.host, &playback_timestamp);
    if (!print_timestamp)
      fprintf(stderr, "4");
    else
      fprintf(stderr, "%lf  (4*)\n", laser4remission.timestamp);
    if(publish) {
      wait_for_timestamp(playback_timestamp);
      err = IPC_publishData(CARMEN_LASER_LASER4_REMISSION_NAME, &laser4remission);
    }
    free(laser4remission.range);
    free(laser4remission.remission);
    return 1;
  }
// *** REI - END *** //

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
    sleep_ipc(0.1);
  }
}

void usage(char *fmt, ...) 
{
  va_list args;
  
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  
  fprintf(stderr, "Usage: playback filename <args>\n");
  fprintf(stderr, "\t-fast         - ignore timestamps.\n");
  fprintf(stderr, "\t-paused       - start paused.\n");
  fprintf(stderr, "\t-syncto tag   - start from the tag tag\n");
  fprintf(stderr, "\t-timestamp    - print current timestamp to stderr\n");
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
      if (strncmp(argv[index], "-fast", 5) == 0)
	fast = 1;
      if (strncmp(argv[index], "-timestamp", 10) == 0)
	print_timestamp = 1;
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
