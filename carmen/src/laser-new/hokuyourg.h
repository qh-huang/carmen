#ifndef _HOKUYO_URG_H_
#define _HOKUYO_URG_H_
#include <math.h>
#include <sys/time.h>
#include "carmen_laser_device.h"

typedef struct {
	int startStep, endStep, cluster, txretries, dataretries;
	int fd;
} HokuyoURG;


void hokuyo_init(HokuyoURG *h);
double hokuyo_getStep(HokuyoURG* h, int cluster);
double hokuyo_getStartAngle(HokuyoURG* h, int m);
int  hokuyo_open(HokuyoURG *h, const char* filename);
int  hokuyo_close(HokuyoURG *h);
int  hokuyo_getVersion(HokuyoURG *h, char*  result, int retries);
int  hokuyo_laserOn(HokuyoURG *h, int retries);
int  hokuyo_laserOff(HokuyoURG *h, int retries);
int  hokuyo_getReading(HokuyoURG *h, unsigned short* result, struct timeval* tv, 
int min, int max, int cluster, int retries, int reading_retries );
#endif


