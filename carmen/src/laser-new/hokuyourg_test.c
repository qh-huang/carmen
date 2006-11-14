#include <stdio.h>
#include <stdlib.h>
#include "hokuyourg.h"
#include <string.h>

int main(int argc, const char** argv){
  HokuyoURG urg;
  char buf[1024];
  char device[1024];
  int i;
  
  fprintf(stderr,"Info: If the laser does not start, simply restart it!\n\n");

  strcpy(device, "/dev/ttyACM0");
  if (argc==2){
    strncpy(device,argv[1], 1023);
  }
  
  hokuyo_init(&urg);
  if (! hokuyo_open(&urg,device)){
    fprintf(stderr,"Error in opening file\n");
    exit(-1);
  }
  fprintf(stderr,"querying version\n");
  hokuyo_getVersion(&urg, buf, -1);
  fprintf(stderr,"version %s\n",buf); 	
  
  fprintf(stderr,"switching on laser \n"); 	
  int retVal=hokuyo_laserOn(&urg,-1);
  if(retVal)
    fprintf(stderr,"Success\n");
  else {
    fprintf(stderr,"Error\n");
    exit (-1);
  }
  
  fprintf(stderr,"reading data\n");
  for (i=0; i<100000; i++){
    struct timeval timestamp;
    unsigned short readings[1024];
    int size=hokuyo_getReading(&urg, readings, &timestamp, -1, -1, -1, -1, -1);
    if (size){
      double angle=hokuyo_getStartAngle(&urg,-1);
      double step=hokuyo_getStep(&urg,-1);
      int j;
      printf ("set size ratio -1 \n" );
      printf ("plot [-2000:2000][-2000:2000] '-' w l \n");
      fflush(stdout);
      for (j=0; j<size; j++){
	fprintf(stderr,".");
	if (readings[j]>=20) {
	  printf ("0 0\n");
	  printf ("%lf %lf\n\n", cos(angle)*readings[j], sin(angle)*readings[j]);
	}
	angle +=step;
      }
      printf ("e\n");
      fflush(stdout);
    }
  }
  return 0;
}
