
#include <carmen/carmen.h>
#include <carmen/carmen_stdio.h>
#include <carmen/robot_interface.h>
#include <carmen/readlog.h>

long int carmen_logfile_uncompressed_length(carmen_FILE *infile)
{
  unsigned char buffer[10000];
  long int log_bytes = 0;
  int nread;
  struct stat stat_buf;

  if(!infile->compressed) {
    /* compute total length of logfile */
    carmen_fseek(infile, 0L, SEEK_SET);
    log_bytes = 0;
    do {
      nread = carmen_fread(buffer, 1, 10000, infile);
      log_bytes += nread;
    } while(nread > 0);
    carmen_fseek(infile, 0L, SEEK_SET);
    return log_bytes;
  } 
  else {
    /* report compressed size for compressed files */
    fstat(fileno(infile->fp), &stat_buf);
    return stat_buf.st_size;
  }
}

carmen_logfile_index_p carmen_logfile_index_messages(carmen_FILE *infile)
{
  carmen_logfile_index_p index;
  int i, found_linebreak = 1, nread, total_bytes, max_messages, read_count = 0;
  int file_length = 0, file_position = 0;
  unsigned char buffer[10000];

  /* allocate and initialize an index */
  index = (carmen_logfile_index_p)calloc(1, sizeof(carmen_logfile_index_t));
  carmen_test_alloc(index);

  /* compute the total length of the uncompressed logfile. */
  fprintf(stderr, "\n\rIndexing messages (0%%)    ");
  file_length = carmen_logfile_uncompressed_length(infile);

  /* mark the start of all messages */
  index->num_messages = 0;
  max_messages = 10000;
  index->offset = (long int *)calloc(max_messages, sizeof(long int));
  carmen_test_alloc(index->offset);

  carmen_fseek(infile, 0L, SEEK_SET);

  total_bytes = 0;
  do {
    nread = carmen_fread(buffer, 1, 10000, infile);
    read_count++;
    if(read_count % 1000 == 0) {
      if(!infile->compressed)
	file_position = total_bytes + nread;
      else
	file_position = lseek(fileno(infile->fp), 0, SEEK_CUR);
      fprintf(stderr, "\rIndexing messages (%.0f%%)      ", 
	      ((float)file_position) / file_length * 100.0);
    }

    if(nread > 0) {
      for(i = 0; i < nread; i++) {
        if(found_linebreak && buffer[i] != '\r') {
          found_linebreak = 0;
	  if(index->num_messages == max_messages) {
	    max_messages += 10000;
	    index->offset = (long int *)realloc(index->offset, max_messages *
						sizeof(long int));
	    carmen_test_alloc(index->offset);
	  }
	  index->offset[index->num_messages] = total_bytes + i;
	  index->num_messages++;
        }
        if(buffer[i] == '\n')
          found_linebreak = 1;
      }
      total_bytes += nread;
    }
  } while(nread > 0);
  fprintf(stderr, "\rIndexing messages (100%%) - %d messages found.      \n",
	  index->num_messages);
  carmen_fseek(infile, 0L, SEEK_SET);
  index->current_position = 0;
  return index;
}

void carmen_logfile_free_index(carmen_logfile_index_p* pindex) {
  if (pindex == NULL) 
    return;

  if ( (*pindex) == NULL) 
    return;
    
  if ( (*pindex)->offset != NULL) {
    free( (*pindex)->offset);
  }
  free(*pindex);
  (*pindex) = NULL;
}

int carmen_logfile_eof(carmen_logfile_index_p index)
{
  if(index->current_position >= index->num_messages - 1)
    return 1;
  else
    return 0;
}

float carmen_logfile_percent_read(carmen_logfile_index_p index)
{
  return index->current_position / (float)index->num_messages;
}

int carmen_logfile_read_line(carmen_logfile_index_p index, carmen_FILE *infile,
			     int message_num, int max_line_length, char *line)
{
  int nread;
  
  /* are we moving sequentially through the logfile?  If not, fseek */
  if(message_num != index->current_position) {
    index->current_position = message_num;
    carmen_fseek(infile, index->offset[index->current_position], SEEK_SET);
  }

  /* check maximum line length */
  if(index->offset[index->current_position + 1] - 
     index->offset[index->current_position] >= max_line_length)
    carmen_die("Error: exceed maximum line length.\n");

  /* read the line of the logfile */
  nread = carmen_fread(line, 1, index->offset[index->current_position + 1] - 
		       index->offset[index->current_position], infile);
  line[nread] = '\0';
  index->current_position++;
  return nread;
}

int carmen_logfile_read_next_line(carmen_logfile_index_p index, carmen_FILE *infile,
				  int max_line_length, char *line)
{
  return carmen_logfile_read_line(index, infile, 
				  index->current_position, 
				  max_line_length, line);
}


int first_wordlength(char *str)
{
  return strchr(str, ' ') - str;
}

void copy_host_string(char **host, char **string)
{
  int l;

  while(*string[0] == ' ')
    *string += 1;                           /* advance past spaces */
  l = first_wordlength(*string) + 1;
  if(*host != NULL)
    free(*host);
  *host = (char *)calloc(1, l);
  carmen_test_alloc(*host);
  strncpy(*host, *string, l - 1);
  (*host)[l] = '\0';
  *string += l;
}

char clf_read_char_helper(char* str, char** strret) {
  /* remove spaces */
  char ret;
  while ( isspace(str[0]) )
    str++;  
  /* if end of string return last valid char */
  if (str[0] == '\0') {
    str--;
  }
  /* return char */
  ret = str[0];
  /* goto next character*/
  str++;
  /* set return string */
  *strret = str;
  return ret;
}


#define CLF_READ_DOUBLE(str) strtod(*(str), (str))
#define CLF_READ_INT(str) (int)strtol(*(str), (str), 10)
#define CLF_READ_CHAR(str) clf_read_char_helper(*(str), (str))


char *carmen_string_to_base_odometry_message(char *string,
					     carmen_base_odometry_message
					     *odometry)
{
  char *current_pos = string;

  if (strncmp(current_pos, "ODOM", 4) == 0)
    current_pos = carmen_next_word(current_pos); 
  
  odometry->x = CLF_READ_DOUBLE(&current_pos);
  odometry->y = CLF_READ_DOUBLE(&current_pos);
  odometry->theta = CLF_READ_DOUBLE(&current_pos);
  odometry->tv = CLF_READ_DOUBLE(&current_pos);
  odometry->rv = CLF_READ_DOUBLE(&current_pos);
  odometry->acceleration = CLF_READ_DOUBLE(&current_pos);
  odometry->timestamp = CLF_READ_DOUBLE(&current_pos);
  copy_host_string(&odometry->host, &current_pos);
  return current_pos;
}

char *carmen_string_to_simulator_truepos_message(char *string,
						 carmen_simulator_truepos_message *truepos)
{
  char *current_pos = string;

  if (strncmp(current_pos, "TRUEPOS", 7) == 0)
    current_pos = carmen_next_word(current_pos); 

 
  truepos->truepose.x = CLF_READ_DOUBLE(&current_pos);
  truepos->truepose.y = CLF_READ_DOUBLE(&current_pos);
  truepos->truepose.theta = CLF_READ_DOUBLE(&current_pos);
  truepos->odometrypose.x = CLF_READ_DOUBLE(&current_pos);
  truepos->odometrypose.y = CLF_READ_DOUBLE(&current_pos);
  truepos->odometrypose.theta = CLF_READ_DOUBLE(&current_pos);
  truepos->timestamp = CLF_READ_DOUBLE(&current_pos);
  copy_host_string(&truepos->host, &current_pos);
  return current_pos;
}

double carmen_laser_guess_fov(int num_beams)
{
  if (num_beams == 181)
    return M_PI;                  /* 180 degrees */
  else if (num_beams == 180)
    return M_PI / 180.0 * 179.0;  /* 179 degrees (last beam ignored)*/
  else if (num_beams == 361)
    return M_PI;                  /* 180 degrees */
  else if (num_beams == 360)
    return M_PI / 180.0 * 179.5 ; /* 179.5 degrees (last beam ignored)*/
  else if (num_beams == 401)
    return M_PI / 180.0 * 100.0 ; /* 100.0 degrees */
  else if (num_beams == 400)
    return M_PI / 100.0 * 99.75 ; /* 99.75 degrees (last beam ignored)*/
  else
    return M_PI;                  /* assume 180 degrees */
}

double carmen_laser_guess_angle_increment(int num_beams)
{
  if (num_beams == 181 || num_beams == 180)
    return M_PI / 180.0; /* 1 degree = M_PI/180 */
  else if (num_beams == 361 || num_beams == 360)
    return M_PI / 360.0;  /* 0.5 degrees = M_PI/360 */
  else if (num_beams == 401 || num_beams == 400)
    return M_PI / 720.0;  /* 0.25 degrees = M_PI/720 */
  else
    return carmen_laser_guess_fov(num_beams) /
      ((double) (num_beams-1));
}

char *carmen_string_to_laser_laser_message_orig(char *string,
						carmen_laser_laser_message
						*laser)
{
  char *current_pos = string;
  int i, num_readings;

  if (strncmp(current_pos, "LASER", 5) == 0)
    current_pos = carmen_next_word(current_pos); 
 

  num_readings = CLF_READ_INT(&current_pos);
  if(laser->num_readings != num_readings) {
    laser->num_readings = num_readings;
    laser->range = (float *)realloc(laser->range, laser->num_readings * 
				    sizeof(float));
    carmen_test_alloc(laser->range);
  }
  for(i = 0; i < laser->num_readings; i++)
    laser->range[i] = CLF_READ_DOUBLE(&current_pos);
  laser->timestamp = CLF_READ_DOUBLE(&current_pos);
  copy_host_string(&laser->host, &current_pos);

  /* fill in remission with nothing */
  laser->num_remissions = 0;
  laser->remission = NULL;

  /* guess at fields */
  laser->config.laser_type = SICK_LMS;
  laser->config.fov = carmen_laser_guess_fov(laser->num_readings);
  laser->config.start_angle = -M_PI / 2.0;
  laser->config.angular_resolution = 
    carmen_laser_guess_angle_increment(laser->num_readings);
  laser->config.maximum_range = 80.0;
  laser->config.accuracy = 0.01;
  laser->config.remission_mode = 0;

  return current_pos;
}

char *carmen_string_to_robot_laser_message_orig(char *string,
						carmen_robot_laser_message
						*laser)
{
  char *current_pos = string;
  int i, num_readings;

  if (strncmp(current_pos, "FLASER", 6) == 0 || 
      strncmp(current_pos, "RLASER", 6) == 0)
    current_pos = carmen_next_word(current_pos); 

  num_readings = CLF_READ_INT(&current_pos);
  if(laser->num_readings != num_readings) {
    laser->num_readings = num_readings;
    laser->range = (float *)realloc(laser->range, laser->num_readings * 
				    sizeof(float));
    carmen_test_alloc(laser->range);
    laser->tooclose = (char *)realloc(laser->tooclose, laser->num_readings);
    carmen_test_alloc(laser->tooclose);
  }
  for(i = 0; i < laser->num_readings; i++)
    laser->range[i] = CLF_READ_DOUBLE(&current_pos);

  laser->laser_pose.x = CLF_READ_DOUBLE(&current_pos);
  laser->laser_pose.y = CLF_READ_DOUBLE(&current_pos);
  laser->laser_pose.theta = CLF_READ_DOUBLE(&current_pos);
  laser->robot_pose.x = CLF_READ_DOUBLE(&current_pos);
  laser->robot_pose.y = CLF_READ_DOUBLE(&current_pos);
  laser->robot_pose.theta = CLF_READ_DOUBLE(&current_pos);
  laser->timestamp = CLF_READ_DOUBLE(&current_pos);
  copy_host_string(&laser->host, &current_pos);

  /* fill in remission with nothing */
  laser->num_remissions = 0;
  laser->remission = NULL;

  /* guess at fields */
  laser->config.laser_type = SICK_LMS;
  laser->config.fov = carmen_laser_guess_fov(laser->num_readings);
  laser->config.start_angle = -M_PI / 2.0;
  laser->config.angular_resolution = 
    carmen_laser_guess_angle_increment(laser->num_readings);
  laser->config.maximum_range = 80.0;
  laser->config.accuracy = 0.01;
  laser->config.remission_mode = 0;

  return current_pos;
}

char *carmen_string_to_laser_laser_message(char *string,
					   carmen_laser_laser_message *laser)
{
  char *current_pos = string;
  int i, num_readings, num_remissions;

  if (strncmp(current_pos, "RAWLASER", 6) == 0)
    current_pos = carmen_next_word(current_pos); 

  laser->config.laser_type = CLF_READ_INT(&current_pos);
  laser->config.start_angle = CLF_READ_DOUBLE(&current_pos);
  laser->config.fov = CLF_READ_DOUBLE(&current_pos);
  laser->config.angular_resolution = CLF_READ_DOUBLE(&current_pos);
  laser->config.maximum_range = CLF_READ_DOUBLE(&current_pos);
  laser->config.accuracy = CLF_READ_DOUBLE(&current_pos);
  laser->config.remission_mode = CLF_READ_INT(&current_pos);

  num_readings = CLF_READ_INT(&current_pos);
  if(laser->num_readings != num_readings) {
    laser->num_readings = num_readings;
    laser->range = (float *)realloc(laser->range, laser->num_readings * 
				    sizeof(float));
    carmen_test_alloc(laser->range);
  }
  for(i = 0; i < laser->num_readings; i++)
    laser->range[i] = CLF_READ_DOUBLE(&current_pos);

  num_remissions = CLF_READ_INT(&current_pos);
  if(laser->num_remissions != num_remissions) {
    laser->num_remissions = num_remissions;
    laser->remission = (float *)realloc(laser->remission, 
					laser->num_remissions * sizeof(float));
    carmen_test_alloc(laser->remission);
  }
  for(i = 0; i < laser->num_remissions; i++)
    laser->remission[i] = CLF_READ_DOUBLE(&current_pos);

  laser->timestamp = CLF_READ_DOUBLE(&current_pos);
  copy_host_string(&laser->host, &current_pos);

  return current_pos;
}

char *carmen_string_to_robot_laser_message(char *string,
					   carmen_robot_laser_message *laser)
{
  char *current_pos = string;
  int i, num_readings, num_remissions;

  if (strncmp(current_pos, "ROBOTLASER", 10) == 0)
    current_pos = carmen_next_word(current_pos); 

  laser->config.laser_type = CLF_READ_INT(&current_pos);
  laser->config.start_angle = CLF_READ_DOUBLE(&current_pos);
  laser->config.fov = CLF_READ_DOUBLE(&current_pos);
  laser->config.angular_resolution = CLF_READ_DOUBLE(&current_pos);
  laser->config.maximum_range = CLF_READ_DOUBLE(&current_pos);
  laser->config.accuracy = CLF_READ_DOUBLE(&current_pos);
  laser->config.remission_mode = CLF_READ_INT(&current_pos);

  num_readings = CLF_READ_INT(&current_pos);
  if(laser->num_readings != num_readings) {
    laser->num_readings = num_readings;
    laser->range = (float *)realloc(laser->range, laser->num_readings * 
				    sizeof(float));
    carmen_test_alloc(laser->range);
  }
  for(i = 0; i < laser->num_readings; i++)
    laser->range[i] = CLF_READ_DOUBLE(&current_pos);

  num_remissions = CLF_READ_INT(&current_pos);
  if(laser->num_remissions != num_remissions) {
    laser->num_remissions = num_remissions;
    laser->remission = (float *)realloc(laser->remission, 
					laser->num_remissions * sizeof(float));
    carmen_test_alloc(laser->remission);
  }
  for(i = 0; i < laser->num_remissions; i++)
    laser->remission[i] = CLF_READ_DOUBLE(&current_pos);

  laser->laser_pose.x = CLF_READ_DOUBLE(&current_pos);
  laser->laser_pose.y = CLF_READ_DOUBLE(&current_pos);
  laser->laser_pose.theta = CLF_READ_DOUBLE(&current_pos);
  laser->robot_pose.x = CLF_READ_DOUBLE(&current_pos);
  laser->robot_pose.y = CLF_READ_DOUBLE(&current_pos);
  laser->robot_pose.theta = CLF_READ_DOUBLE(&current_pos);

  laser->tv = CLF_READ_DOUBLE(&current_pos);
  laser->rv = CLF_READ_DOUBLE(&current_pos);
  laser->forward_safety_dist = CLF_READ_DOUBLE(&current_pos);
  laser->side_safety_dist = CLF_READ_DOUBLE(&current_pos);
  laser->turn_axis = CLF_READ_DOUBLE(&current_pos);

  laser->timestamp = CLF_READ_DOUBLE(&current_pos);
  copy_host_string(&laser->host, &current_pos);

  return current_pos;
}



char *carmen_string_to_gps_gpgga_message(char *string,
					 carmen_gps_gpgga_message *gps)
{
  char *current_pos = string;

  if (strncmp(current_pos, "GPSNMEA", 7) == 0)
    current_pos = carmen_next_word(current_pos); 

  gps->nr             = CLF_READ_INT(&current_pos);
  gps->utc            = CLF_READ_DOUBLE(&current_pos);
  gps->latitude       = CLF_READ_DOUBLE(&current_pos);
  gps->lat_orient     = CLF_READ_CHAR(&current_pos);
  gps->longitude      = CLF_READ_DOUBLE(&current_pos);
  gps->long_orient    = CLF_READ_CHAR(&current_pos);
  gps->gps_quality    = CLF_READ_INT(&current_pos);
  gps->num_satellites = CLF_READ_INT(&current_pos);
  gps->hdop           = CLF_READ_DOUBLE(&current_pos);
  gps->sea_level      = CLF_READ_DOUBLE(&current_pos);
  gps->altitude       = CLF_READ_DOUBLE(&current_pos);
  gps->geo_sea_level  = CLF_READ_DOUBLE(&current_pos);
  gps->geo_sep        = CLF_READ_DOUBLE(&current_pos);
  gps->data_age       = CLF_READ_INT(&current_pos);

  gps->timestamp      = CLF_READ_DOUBLE(&current_pos);
  copy_host_string(&gps->host, &current_pos);

  return current_pos;
}


