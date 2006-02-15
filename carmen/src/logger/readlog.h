/** @addtogroup logger libreadlog **/
// @{

/** 
 * \file readlog.h 
 * \brief Library for reading log files. 
 *
 * This library should be used to read logfiles. It uses an index
 * structure in order to go thourgh the individual messages of the
 * carmen log file. This library furthermore provides conversion
 * functions that convert strings (representing a message in the
 * carmen log file format) into the corresponding
 * laser/odometry/etc. message.
 **/

#ifndef CARMEN_READLOG_H
#define CARMEN_READLOG_H

#ifdef __cplusplus
extern "C" {
#endif

/** Index structure used to process a logfile. **/
typedef struct {
  int num_messages;     /**< Number of message in the file. **/
  int current_position; /**< Iterator to move through the file. **/
  long int *offset;     /**< Array of indices to the messages. **/
} carmen_logfile_index_t, *carmen_logfile_index_p;

/** Builds the index structure used for parsing a carmen log file. 
 * @param infile  A pointer to a CARMEN_FILE.
 * @returns A pointer to the newly created index structure.
 **/
carmen_logfile_index_p carmen_logfile_index_messages(carmen_FILE *infile);

/** Frees an index structure **/
void carmen_logfile_free_index(carmen_logfile_index_p* pindex);

/** End of file reached in the index structure. **/
int carmen_logfile_eof(carmen_logfile_index_p index);

/** Percentage of the how much data has been read **/
float carmen_logfile_percent_read(carmen_logfile_index_p index);

/** Reads a line from the log file using the index structre .
 * @param index A pointer to the index structure correspondinf to infile.
 * @param infile A Carmen file pointer to the log file.
 * @param message_num The index of the message to read.
 * @param max_line_length The size of line.
 * @param line A pointer to the address where the line is written to. 
 * @returns Number of read bytes.
 **/
int carmen_logfile_read_line(carmen_logfile_index_p index, carmen_FILE *infile,
			     int message_num, int max_line_length, char *line);

/** Reads the next line from the log file using the index structre.
 * @param index A pointer to the index structure correspondinf to infile.
 * @param infile A Carmen file pointer.
 * @param max_line_length allocated memory in line.
 * @param line A pointer to the address where the line is written to.
 * @returns Number of read bytes.
 **/
int carmen_logfile_read_next_line(carmen_logfile_index_p index, carmen_FILE *infile,
				  int max_line_length, char *line);

/** Converts the string to an odometry message.
 * @param string A string describing the message in the carmen logfile format.
 * @param odometru A pointer to the (allocated) structure where the message should be written to.
 * @returns A pointer to the character up to which the string has been parsed. 
 * In case there are no errors and the string is a line read from a carmen log
 * file, this string points to the relative timestamp (written by logger).
 **/
char *carmen_string_to_base_odometry_message(char *string,
					     carmen_base_odometry_message
					     *odometry);

/** Converts the string to a truepos message. 
 * @param string A string describing the message in the *old* carmen logfile format.
 * @param truepos A pointer to the (allocated) structure where the message should be written to.
 * @returns A pointer to the character up to which the string has been parsed. 
 * In case there are no errors and the string is a line read from a carmen log
 * file, this string points to the relative timestamp (written by logger).
 **/
char *carmen_string_to_simulator_truepos_message(char *string,
						 carmen_simulator_truepos_message *truepos);

/** Converts the string to an old robot_laser_message.
 * @param string A string describing the message in the carmen logfile format.
 * @param laser A pointer to the (allocated) structure where the message should be written to.
 * @returns A pointer to the character up to which the string has been parsed. 
 * In case there are no errors and the string is a line read from a carmen log
 * file, this string points to the relative timestamp (written by logger).
 **/
char *carmen_string_to_robot_laser_message_orig(char *string,
						carmen_robot_laser_message
						*laser);

/** Converts the string to an old laser_laser_message. 
 * @param string A string describing the message in the *old* carmen logfile format.
 * @param laser A pointer to the (allocated) structure where the message should be written to.
 * @returns A pointer to the character up to which the string has been parsed. 
 * In case there are no errors and the string is a line read from a carmen log
 * file, this string points to the relative timestamp (written by logger).
 **/
char *carmen_string_to_laser_laser_message_orig(char *string,
						carmen_laser_laser_message
						*laser);

/** Converts the string to a (new) robot_laser_message. 
 * @param string A string describing the message in the carmen logfile format.
 * @param laser A pointer to the (allocated) structure where the message should be written to.
 * @returns A pointer to the character up to which the string has been parsed. 
 * In case there are no errors and the string is a line read from a carmen log
 * file, this string points to the relative timestamp (written by logger).
 **/
char *carmen_string_to_robot_laser_message(char *string,
					   carmen_robot_laser_message *laser);

/** Converts the string to a (new) laser_laser_message. 
 * @param string A string describing the message in the carmen logfile format.
 * @param laser A pointer to the (allocated) structure where the message should be written to.
 * @returns A pointer to the character up to which the string has been parsed. 
 * In case there are no errors and the string is a line read from a carmen log
 * file, this string points to the relative timestamp (written by logger).
 **/
char *carmen_string_to_laser_laser_message(char *string,
					   carmen_laser_laser_message *laser);


/** Converts the string to a gps-nmea-rmc message. 
 * @param string A string describing the message in the carmen logfile format.
 * @param gps_msg A pointer to the (allocated) structure where the message should be written to.
 * @returns A pointer to the character up to which the string has been parsed. 
 * In case there are no errors and the string is a line read from a carmen log
 * file, this string points to the relative timestamp (written by logger).
 **/
char *carmen_string_to_gps_gprmc_message(char *string,
					 carmen_gps_gprmc_message *gps_msg);

/** Converts the string to a gps-nmea-gga message. 
 * @param string A string describing the message in the carmen logfile format.
 * @param gps_msg A pointer to the (allocated) structure where the message should be written to.
 * @returns A pointer to the character up to which the string has been parsed. 
 * In case there are no errors and the string is a line read from a carmen log
 * file, this string points to the relative timestamp (written by logger).
 **/
char *carmen_string_to_gps_gpgga_message(char *string,
					 carmen_gps_gpgga_message *gps_msg);



#ifdef __cplusplus
}
#endif

#endif

// @}
