#ifndef LLCLIB_H
#define LLCLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

/* open a serial port. dev is the unix device file to open, eg. "/dev/ttyS0".
 * return value: a file descriptor used as an argument to all other commands
 * below.
 */
int open_serialport( char *dev );

/* close a serial port opened previously with open_serialport().
 */
void close_serialport( int fd );

/* returns true only if the LLC responds to a status query within a
 * period of approximately 100ms
 */
int llc_check_alive( int fd );

/* throws away all data currently queued by serial port receiver.
 */
void llc_flush_input( int fd );

/* reads 'len' bytes from the serial port into the array 'result'.
 */
int llc_get_result( int fd, unsigned char* result, int len );

/* reads up to 'max_len" bytes from the seral port into 'result'.
 * the function returns when 'max_len' bytes have been read or 
 * 'max_time' microseconds have elapsed.
 */
int llc_get_result_with_timeout( int fd, unsigned char* result, int max_len, int max_time );

/* returns the current LLC encoder reading as a 2-byte unsigned short.
 */
unsigned short llc_read_encoder( int fd );

/* returns the current LLC encoder velocity reading as a single byte.
 */
unsigned char llc_read_encoder_vel( int fd );

/* returns the current LLC analog input reading as a single byte.
 */
unsigned char llc_read_analog( int fd );

/* resets the current encoder position to 0.
 */
int llc_reset_encoder( int fd );

/* pulse the selected pulse output for the given duration.
 * the output must be 0, 1, 2, or 3.
 * the actual pulse duration is 2.5ms for each unit of duration provided.
 * the maximum duration is 255, which corresponds to 637.5ms.
 * a nonzero error value is returned if the output number or pulse duration
 * is not valid.
 */
int llc_pulse_output( int fd, int output, int duration );

#endif
