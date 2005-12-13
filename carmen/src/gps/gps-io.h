#define DEFAULT_GPS_PORT      "/dev/ttyS0"
#define DEFAULT_GPS_BAUD      4800
#define DEFAULT_GPS_PARITY    NO
#define DEFAULT_GPS_DATABITS  8
#define DEFAULT_GPS_STOPBITS  1
#define DEFAULT_GPS_HWF       1
#define DEFAULT_GPS_SWF       0

void  DEVICE_set_params( SerialDevice dev );

void  DEVICE_set_baudrate( SerialDevice dev, int brate );

int   DEVICE_connect_port( SerialDevice *dev );

int   DEVICE_send( SerialDevice dev, unsigned char *cmd, int len );

int   DECVICE_recieve( SerialDevice dev, unsigned char *cmd, int *len );

int   DEVICE_read_data( SerialDevice dev );

int   DEVICE_bytes_waiting( int sd );

void  DEVICE_init_params( SerialDevice *p );

#define BUFFER_LENGTH         512

