typedef struct {
  int    set_brake;        /* 0: off, 1: on */
  double timestamp;
  char host[10];
} carmen_rflex_brake_message;

#define      CARMEN_RFLEX_BRAKE_NAME       "carmen_rflex_brake"
#define      CARMEN_RFLEX_BRAKE_FMT        "{int,double,[char:10]}"

