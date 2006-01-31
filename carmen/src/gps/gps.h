#define MAX_NAME_LENGTH                256
#define MAX_COMMAND_LENGTH             256

#define EPSILON                     0.0001

#define TIMEOUT                         -1
#define WRONG                            0
#define OK                               1


enum PARITY_TYPE   { NO, EVEN, ODD };

typedef struct {
  char                       ttyport[MAX_NAME_LENGTH];
  int                        baud;
  enum PARITY_TYPE           parity;
  FILE *                     fp;
  int                        fd;
  int                        databits;
  int                        stopbits;
  int                        hwf;
  int                        swf;
} SerialDevice;

extern carmen_gps_gpgga_message    * carmen_extern_gpgga_ptr;
extern carmen_gps_gprmc_message    * carmen_extern_gprmc_ptr;

int    carmen_gps_parse_data( char * line, int num_chars );


