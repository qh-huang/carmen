#ifndef CARMEN_READLOG_H
#define CARMEN_READLOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int num_messages, current_position;
  long int *offset;
} carmen_logfile_index_t, *carmen_logfile_index_p;

carmen_logfile_index_p carmen_logfile_index_messages(carmen_FILE *infile);

void carmen_logfile_free_index(carmen_logfile_index_p* pindex);

int carmen_logfile_eof(carmen_logfile_index_p index);

float carmen_logfile_percent_read(carmen_logfile_index_p index);

int carmen_logfile_read_line(carmen_logfile_index_p index, carmen_FILE *infile,
			     int message_num, int max_line_length, char *line);

int carmen_logfile_read_next_line(carmen_logfile_index_p index, carmen_FILE *infile,
				  int max_line_length, char *line);

char *carmen_string_to_base_odometry_message(char *string,
					     carmen_base_odometry_message
					     *odometry);

char *carmen_string_to_simulator_truepos_message(char *string,
						 carmen_simulator_truepos_message *truepos);

char *carmen_string_to_robot_laser_message_orig(char *string,
						carmen_robot_laser_message
						*laser);

char *carmen_string_to_laser_laser_message_orig(char *string,
						carmen_laser_laser_message
						*laser);

char *carmen_string_to_robot_laser_message(char *string,
					   carmen_robot_laser_message *laser);

char *carmen_string_to_laser_laser_message(char *string,
					   carmen_laser_laser_message *laser);

char *carmen_string_to_gps_gpgga_message(char *string,
					 carmen_gps_gpgga_message *gps);


#ifdef __cplusplus
}
#endif

#endif
