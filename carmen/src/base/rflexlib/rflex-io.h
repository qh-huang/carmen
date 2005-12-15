long  bytesWaiting( int sd );

void  DEVICE_set_params( Device dev );

void  DEVICE_set_baudrate( Device dev, int brate );

int   DEVICE_connect_port( Device *dev );

#define BUFFER_LENGTH         512

int   writeData( int fd, unsigned char *buf, int nChars );

int   waitForAnswer( int fd, unsigned char *buf, int *len );
