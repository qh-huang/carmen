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

#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define CARMEN_MAJOR_VERSION 0
#define CARMEN_MINOR_VERSION 4
#define CARMEN_REVISION 6

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Useful macros */

typedef enum {CARMEN_UNSUBSCRIBE, CARMEN_SUBSCRIBE_LATEST, CARMEN_SUBSCRIBE_ALL} carmen_subscribe_t;
typedef enum {CARMEN_SIMULATOR_RANDOM_OBJECT, CARMEN_SIMULATOR_LINE_FOLLOWER,
	      CARMEN_SIMULATOR_OTHER_ROBOT} carmen_simulator_object_t;

typedef void (*carmen_handler_t)(void *);

typedef struct {
  double x;
  double y;
  double theta;
} carmen_point_t, *carmen_point_p;

typedef struct {
  double x;
  double y;
  double theta;
  double t_vel;
  double r_vel;
} carmen_traj_point_t, *carmen_traj_point_p;

typedef struct {
  double max_r_vel;
  double max_t_vel;
  double acceleration;
  double curvature;
  double approach_dist;
  double side_dist;
  double length;
  double width;
  double reaction_time;
  int allow_rear_motion;
  int rectangular;
} carmen_robot_config_t;

typedef struct {
  int X1, Y1;
  int X2, Y2;
  int Increment;
  int UsingYIndex;
  int DeltaX, DeltaY;
  int DTerm;
  int IncrE, IncrNE;
  int XIndex, YIndex;
  int Flipped;
} carmen_bresenham_param_t;

typedef void (*carmen_usage_func)(char *fmt, ...);

typedef struct {
  int length;
  int capacity;
  int entry_size;
  void *list;
} carmen_list_t;

#define carmen_red_code "[31;1m"
#define carmen_blue_code "[34;1m"
#define carmen_normal_code "[0m"

#define carmen_time_code(code, str) { double time_code_t1, time_code_t2; time_code_t1 = carmen_get_time_ms(); code; time_code_t2 = carmen_get_time_ms(); fprintf(stderr, "%-20s : %.2f ms.\n", str, (time_code_t2 - time_code_t1) * 1000.0); }

void carmen_test_ipc(IPC_RETURN_TYPE err, const char *err_msg, const char *ipc_msg);

#define carmen_test_alloc(X) do {if ((void *)(X) == NULL) carmen_die("Out of memory in %s, (%s, line %d).\n", __FUNCTION__, __FILE__, __LINE__); } while (0)

#define carmen_test_ipc_return(ERR, ERR_MSG, IPC_MSG) do {carmen_test_ipc((ERR), (ERR_MSG), (IPC_MSG)); if ((ERR) != IPC_OK) return; } while (0)

#define carmen_test_ipc_return_int(ERR, ERR_MSG, IPC_MSG) do {carmen_test_ipc((ERR), (ERR_MSG), (IPC_MSG)); if ((ERR) != IPC_OK) return -1; } while (0)

#define carmen_test_ipc_return_null(ERR, ERR_MSG, IPC_MSG) do {carmen_test_ipc((ERR), (ERR_MSG), (IPC_MSG)); if ((ERR) != IPC_OK) return NULL; } while (0)

#define carmen_test_ipc_exit(ERR, ERR_MSG, IPC_MSG) do {carmen_test_ipc((ERR), (ERR_MSG), (IPC_MSG)); if ((ERR) != IPC_OK) {fprintf(stderr, "This is a fatal error. Exiting.\n"); exit(-1);} } while (0)

int carmen_find_param(char *lvalue);

int carmen_find_param_pair(char *lvalue);

char *carmen_find_robot_name(int argc, char **argv);

char *carmen_param_pair(char *lvalue);

char *carmen_param_pair_and_remove(char *lvalue);

char *carmen_read_params(char *module_name, int argc, char **argv);

int carmen_num_params(void);

char *carmen_get_param_by_num(int param_index);

int carmen_read_commandline_parameters(int argc, char **argv);

int carmen_process_param_int(char *lvalue, carmen_usage_func usage, int *return_value); 

double carmen_process_param_double(char *lvalue, carmen_usage_func usage, double *return_value);

int carmen_process_param_onoff(char *lvalue, carmen_usage_func usage, int *return_value); 

char *carmen_process_param_string(char *lvalue, carmen_usage_func usage);

char *carmen_process_param_file(char *lvalue, carmen_usage_func usage);

char *carmen_process_param_directory(char *lvalue, carmen_usage_func usage);

char *carmen_extract_filename(char *path);

void carmen_perror(char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void carmen_verbose(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void carmen_warn(char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void carmen_die(char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void carmen_die_syserror(char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void carmen_carp_set_verbose(int verbosity);
int carmen_carp_get_verbose(void);
void carmen_carp_set_output(FILE *output);

extern inline double carmen_get_time_ms(void)
{
  struct timeval tv;
  double t;

  if (gettimeofday(&tv, NULL) < 0) 
    carmen_warn("carmen_get_time_ms encountered error in gettimeofday : %s\n",
	      strerror(errno));
  t = tv.tv_sec + tv.tv_usec/1000000.0;
  return t;
}

char *carmen_get_tenchar_host_name(void);

void carmen_initialize_ipc(char *module_name);
void sleep_ipc(double timeout);
void close_ipc(void);

void carmen_running_average_clear(int which);
void carmen_running_average_add(int which, double x);
double carmen_running_average_report(int which);

/* This weirdness of extern inline is to allow the function to be inlined
   outside the library. Guess what! There's an exact copy of this function in
   global.c as well. <sigh> */

extern inline int carmen_round(double X)
{
  if (X >= 0)
    return (int)(X + 0.5);
  else
    return (int)(X - 0.5);
}

extern inline double carmen_clamp(double X, double Y, double Z) 
{
  if (Y < X)
    return X;
  else if (Y > Z)
    return Z;
  return Y;
}

extern inline int carmen_trunc(double X)
{
  return (int)(X);
}

extern inline double carmen_normalize_theta(double theta)
{
  int multiplier;
  
  if (theta >= -M_PI && theta < M_PI)
    return theta;
  
  multiplier = (int)(theta / (2*M_PI));
  theta = theta - multiplier*2*M_PI;
  if (theta >= M_PI)
    theta -= 2*M_PI;
  if (theta < -M_PI)
    theta += 2*M_PI;

  return theta;
}

extern inline double carmen_radians_to_degrees(double theta)
{
  return (theta * 180.0 / M_PI);
}

extern inline double carmen_degrees_to_radians(double theta)
{
  return (theta * M_PI / 180.0);
}

extern inline double carmen_fmin(double val1, double val2)
{
  if (val2 < val1)
    return val2;
  return val1;
}

extern inline double carmen_fmax(double val1, double val2)
{
  if (val2 > val1)
    return val2;
  return val1;
}

extern inline double carmen_square(double val)
{
	return (val*val);
}

extern inline double carmen_distance_traj(carmen_traj_point_p p1, carmen_traj_point_p p2)
{
  return sqrt((p1->x-p2->x)*(p1->x-p2->x) + (p1->y-p2->y)*(p1->y-p2->y));
}

extern inline double carmen_angle_between(carmen_traj_point_p p1, carmen_traj_point_p p2)
{
  return atan2(p2->y - p1->y, p2->x - p1->x);
}

extern inline double carmen_distance(carmen_point_p p1, carmen_point_p p2) 
{
  return sqrt((p1->x-p2->x)*(p1->x-p2->x) + (p1->y-p2->y)*(p1->y-p2->y));
}


void carmen_get_bresenham_parameters(int p1x, int p1y, int p2x, int p2y, 
				   carmen_bresenham_param_t *params);
void carmen_get_current_point(carmen_bresenham_param_t *params, int *x, int *y);
int carmen_get_next_point(carmen_bresenham_param_t *params);
int carmen_sign(double num);

void carmen_rect_to_polar(double x, double y, double *r, double *theta);
void carmen_init_trig_tables(int size);

unsigned int carmen_generate_random_seed(void);
unsigned int carmen_randomize(int *argc, char ***argv);
void carmen_set_random_seed(unsigned int seed);

int carmen_int_random(int max);
double carmen_uniform_random(double min, double max);
double carmen_gaussian_random(double mean, double std);

extern double *carmen_sin_table, *carmen_cos_table;
extern int carmen_trig_table_size;

int carmen_file_exists(char *filename);
char *carmen_file_extension(char *filename);
char *carmen_file_find(char *filename);
char **carmen_get_search_path(int *num_paths);

void carmen_global_start_progess_bar(char *label);
void carmen_global_end_progess_bar(void);  
void carmen_global_update_progess_bar(int count, int size);

int carmen_strcasecmp (const char *s1, const char *s2);
int carmen_strncasecmp (const char *s1, const char *s2, size_t n);

char *carmen_new_string(const char *fmt, ...);
char *carmen_new_stringv(const char *fmt, va_list ap);

void carmen_print_version(void);

int carmen_parse_sonar_offsets(char *offset_string, carmen_point_p offsets,
			       int num_sonars);

int carmen_terminal_cbreak(int blocking);
int carmen_terminal_restore(void);

carmen_list_t *carmen_list_create(int entry_size, int initial_capacity);
carmen_list_t *carmen_list_create_from_data(int entry_size, int num_elements, 
					    void *data);
carmen_list_t *carmen_list_duplicate(carmen_list_t *list);
void carmen_list_add(carmen_list_t *list, void *entry);
void carmen_list_delete(carmen_list_t *list, int entry_num);
void *carmen_list_get(carmen_list_t *list, int entry_num);
void carmen_list_set(carmen_list_t *list, int entry_num, void *entry);
int carmen_list_length(carmen_list_t *list);
void carmen_list_destroy(carmen_list_t **list);

void carmen_eigs_to_covariance(double theta, double major, double minor,
			       double *vx, double *vxy, double *vy);

#ifdef __cplusplus
}
#endif

#endif
