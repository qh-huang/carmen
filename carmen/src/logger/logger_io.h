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

#ifndef LOGGER_IO_H
#define LOGGER_IO_H

#include <carmen/carmen.h>

extern int carmen_logger_nogz;
extern int carmen_playback_nogz;

#define CARMEN_LOGFILE_HEADER "# CARMEN Logfile"

#ifndef NO_ZLIB

#include <zlib.h>

typedef gzFile carmen_logger_file_p;

#define carmen_logger_fopen(X,Y) (carmen_logger_nogz ? fopen(X,Y) : gzopen(X,Y))
#define carmen_playback_fopen(X,Y) (carmen_playback_nogz ? fopen(X,Y) : gzopen(X,Y))
#define carmen_logger_fgetc(X) (carmen_logger_nogz ? fgetc((FILE*)(X)) : gzgetc(X))
#define carmen_playback_fgetc(X) (carmen_playback_nogz ? fgetc((FILE*)(X)) : gzgetc(X))
#define carmen_logger_feof(X) (carmen_logger_nogz ? feof((FILE*)(X)) : gzeof(X))
#define carmen_playback_feof(X) (carmen_playback_nogz ? feof((FILE*)(X)) : gzeof(X))
#define carmen_logger_fseek(X,Y,Z) (carmen_logger_nogz ? fseek((FILE*)(X),Y,Z) : gzseek(X,Y,Z))
#define carmen_playback_fseek(X,Y,Z) (carmen_playback_nogz ? fseek((FILE*)(X),Y,Z) : gzseek(X,Y,Z))
#define carmen_logger_ftell(X) (carmen_logger_nogz ? ftell((FILE*)(X)) : gztell(X))
#define carmen_playback_ftell(X) (carmen_playback_nogz ? ftell((FILE*)(X)) : gztell(X))
#define carmen_logger_fclose(X) (carmen_logger_nogz ? fclose((FILE*)(X)) : gzclose(X))
#define carmen_playback_fclose(X) (carmen_playback_nogz ? fclose((FILE*)(X)) : gzclose(X))
#define carmen_logger_fread(W,X,Y,Z) (carmen_logger_nogz ? fread(W,X,Y,(FILE*)(Z)) : (unsigned int)gzread(Z,W,X*Y))
#define carmen_playback_fread(W,X,Y,Z) (carmen_playback_nogz ? fread(W,X,Y,(FILE*)(Z)) : (unsigned int)gzread(Z,W,X*Y))
#define carmen_logger_fwrite(W,X,Y,Z) (carmen_logger_nogz ? fwrite(W,X,Y,(FILE*)(Z)) : gzwrite(Z,W,X))
#define carmen_playback_fwrite(W,X,Y,Z) (carmen_playback_nogz ? fwrite(W,X,Y,(FILE*)(Z)) : gzwrite(Z,W,X))
#define carmen_logger_fgets(X,Y,Z) (carmen_logger_nogz ? fgets(X,Y,(FILE*)(Z)) : gzgets(Z,X,Y))
#define carmen_playback_fgets(X,Y,Z) (carmen_playback_nogz ? fgets(X,Y,(FILE*)(Z)) : gzgets(Z,X,Y))
#define carmen_logger_fputc(X,Y) (carmen_logger_nogz ? fputc(X,(FILE*)(Y)) : gzputc(Y,X))
#define carmen_playback_fputc(X,Y) (carmen_playback_nogz ? fputc(X,(FILE*)(Y)) : gzputc(Y,X))

void carmen_logger_fprintf(carmen_logger_file_p stream, const char *format, ...);
void carmen_playback_fprintf(carmen_logger_file_p stream, const char *format, ...);

#else

typedef FILE * carmen_logger_file_p;
#define carmen_logger_fopen fopen
#define carmen_playback_fopen fopen
#define carmen_logger_fgetc fgetc
#define carmen_playback_fgetc fgetc
#define carmen_logger_feof feof
#define carmen_playback_feof feof
#define carmen_logger_fprintf fprintf
#define carmen_playback_fprintf fprintf
#define carmen_logger_fseek fseek
#define carmen_playback_fseek fseek
#define carmen_logger_ftell ftell
#define carmen_playback_ftell ftell
#define carmen_logger_fclose fclose
#define carmen_playback_fclose fclose
#define carmen_logger_fread fread
#define carmen_playback_fread fread
#define carmen_logger_fwrite fwrite
#define carmen_playback_fwrite fwrite
#define carmen_logger_fgets fgets
#define carmen_playback_fgets fgets
#define carmen_logger_fputc fputc
#define carmen_playback_fputc fputc

#endif

void carmen_logger_write_robot_name(char *robot_name, carmen_logger_file_p outfile);
void carmen_logger_write_header(carmen_logger_file_p outfile);
void carmen_logger_write_odometry(carmen_base_odometry_message *odometry, 
				  carmen_logger_file_p outfile, double timestamp);  
void carmen_logger_write_frontlaser(carmen_robot_laser_message *frontlaser, 
				    carmen_logger_file_p outfile, double timestamp);
void carmen_logger_write_rearlaser(carmen_robot_laser_message *rearlaser, 
				   carmen_logger_file_p outfile, double timestamp);

void carmen_logger_write_laser3(carmen_laser_laser_message *laser, carmen_logger_file_p outfile,
				double timestamp);

void carmen_logger_write_laser4(carmen_laser_laser_message *laser, carmen_logger_file_p outfile,
			   double timestamp);


void carmen_logger_write_param(char *module, char *variable, char *value, 
			       double ipc_time, char *hostname, 
			       carmen_logger_file_p outfile, double timestamp);
void carmen_logger_write_sync(carmen_logger_sync_message *sync_message, 
			      carmen_logger_file_p outfile);
void carmen_logger_write_truepos(carmen_simulator_truepos_message *truepos, 
				 carmen_logger_file_p outfile, double timestamp);  

void carmen_logger_write_localize(carmen_localize_globalpos_message *msg, 
				  carmen_logger_file_p outfile, double timestamp);

// *** REI - START *** //
void carmen_logger_write_frontlaser_remission(carmen_laser_remission_message *rem, 
					      carmen_logger_file_p outfile, double timestamp);
void carmen_logger_write_rearlaser_remission(carmen_laser_remission_message *rem, 
					      carmen_logger_file_p outfile, double timestamp);
void carmen_logger_write_laser3_remission(carmen_laser_remission_message *rem, 
					      carmen_logger_file_p outfile, double timestamp);
void carmen_logger_write_laser4_remission(carmen_laser_remission_message *rem, 
					      carmen_logger_file_p outfile, double timestamp);
// *** REI - END *** //
#endif
