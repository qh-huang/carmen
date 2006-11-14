#include "hokuyourg.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 8192
#define CMD_EOF       "\26\n"
#define CMD_VERSION   "V\n"
#define CMD_LASER_ON  "L1\n"
#define CMD_LASER_OFF "L0\n"
#define CMD_ACQUIRE   "G"
#define CMD_RESPONSE_PERIOD        3000
#define READING_ACQUIRE_PERIOD     70000

//helper functions
inline char* skipLine(char* s){
  while(*s!='\n' && *s!='\0')
    s++;
  if (*s=='\n') 
    s++;
  return s;
}

inline int write_cmd(int fd, char* s){
  return write(fd,s,strlen(s));
} 

inline int read_cmd(int fd, char* s, int bufsize){
  char* cbuf=s;
  int lfcount=0;
  int d=cbuf-s;
  while (lfcount<2){
    char* c=cbuf;
    int count=read(fd,cbuf,bufsize);
    if (count>0){
      cbuf+=count;
      for (;c<cbuf;c++){
	if (*c=='\n')
	  lfcount++;
	else
	  lfcount=0;
      }
    }
  }
  *cbuf=0;
  return d;
}

inline int blocking_request(int fd, char* cmd, int retries, char* answer, int bufsize){
  int i;
  for (i=0; i<retries; i++){
    int l=write_cmd(fd,cmd);
    usleep(CMD_RESPONSE_PERIOD);
    read_cmd(fd,answer,bufsize);
    if (!strncmp(cmd,answer,l))
      return 1;
    else{
      write_cmd(fd,CMD_EOF);
    }
  }
  return 0;
}

inline int queryVersion(int fd, int retries, char* s, int bufsize){
  return blocking_request(fd,CMD_VERSION,retries,s,bufsize);
}

inline int laserOn(int fd, int retries, char* s, int bufsize){
  return blocking_request(fd,CMD_LASER_ON,retries,s,bufsize);
}

inline int laserOff(int fd, int retries, char* s, int bufsize){
  return blocking_request(fd,CMD_LASER_OFF,retries,s,bufsize);
}

inline int requestReading(int fd, int retries, int min, int max, int cluster, char* s, int bufsize){
  char cmd[bufsize];
  sprintf(cmd,"%s%03d%03d%02d\n",CMD_ACQUIRE,min,max,cluster);
  return blocking_request(fd,cmd,retries,s,bufsize);
}

inline int waitReading(int fd, struct timeval* tv, int min, int max, int cluster, int retries, int reading_retries, unsigned short* readings, int bufsize){
  char cmd[bufsize];
  char answer[bufsize];
  char* s=answer;
  int i;
  int rcount=0;
  int lcount=0;
  char rhigh=0, rlow=0;

  sprintf(cmd,"%s%03d%03d%02d\n",CMD_ACQUIRE,min,max,cluster);
  for (i=0; i<reading_retries; i++){
    if (!blocking_request(fd,cmd,retries,answer,bufsize))
      return 0;
    if (answer[10]=='0')
      break;
  }
  if (i>=reading_retries)
    return 0;
  gettimeofday(tv,0);
  s=skipLine(s); //skip the command
  s=skipLine(s); //skip the status
  int linefeedFound=0;
  while (1){
    if (linefeedFound && *s=='\n') {
      if (rcount > (max-min)/cluster + 1) // sometimes one get strange readings
	return 0;
      return rcount;
    }
    linefeedFound=(*s=='\n');
    if (! linefeedFound){
      if (! (lcount&0x1)){
	rhigh=*s-0x30;
      } else {
	rlow=(*s)-0x30;
	*readings=(rhigh<<6)+rlow;
	readings++;
	rcount++;
      }
      lcount++;
    } else
      lcount=0;
    s++;
  }
}


void hokuyo_init(HokuyoURG *h){
  h->startStep=0;
  h->endStep=768;
  h->cluster=1;
  h->txretries=20;
  h->dataretries=10;
  h->fd=-1;
}


double hokuyo_getStep(HokuyoURG* h, int c){ 
  return (c==-1) ? (2.*M_PI/1024.)*h->cluster : (2.*M_PI/1024.)*c; 
}

double hokuyo_getStartAngle(HokuyoURG* h, int m) { 
  m = (m==-1)?h->startStep:m; 
  return (-135./180.)*M_PI+hokuyo_getStep(h,1)*m; 
}


int hokuyo_open(HokuyoURG *h, const char* filename){
  h->fd=open(filename, O_RDWR|O_SYNC|O_NOCTTY);
  if (h->fd<=0){
    return 0;
  }
  return 1;
}

int hokuyo_close(HokuyoURG *h){
  if (h->fd>0){
    close(h->fd);	
    h->fd=-1;
    return 1;
  }
  return 0;
}

int hokuyo_getVersion(HokuyoURG *h, char*  result, int r){
  if (h->fd==-1)
    return -1;
  r=(r==-1)?h->txretries:r;
  return queryVersion(h->fd, r ,result, BUFSIZE);
}

int hokuyo_laserOn(HokuyoURG *h, int r){
  char buf[BUFSIZE];
  if (h->fd==-1)
    return 0;
  r=(r==-1)?h->txretries:r;
  int retVal=laserOn(h->fd, r ,buf, BUFSIZE);
  if(retVal){
    return 1;
  } else {
    return 0;
  }
}

int hokuyo_laserOff(HokuyoURG *h, int r){
  char buf[BUFSIZE];
  if (h->fd==-1)
    return 0;
  r=(r==-1)?h->txretries:r;
  int retVal=laserOff(h->fd, r ,buf, BUFSIZE);
  if(retVal){
    return 1;
  } else {
    return 0;
  }
}

int hokuyo_getReading(HokuyoURG *h, unsigned short* result, struct timeval* tv, int min, int max, int c, int r, int d){
  unsigned short readings [BUFSIZE];
  int l = -1;
  int i;
  if (h->fd==-1)
    return -1;
  r   = (r  ==-1) ? h->txretries   : r;
  min = (min==-1) ? h->startStep   : min;
  max = (max==-1) ? h->endStep     : max;
  c   = (c == -1) ? h->cluster     : c;
  d   = (d == -1) ? h->dataretries : d;
  l = waitReading(h->fd, tv, min, max, c, r, d, readings, BUFSIZE);
  for (i=0; i<l; i++)
    result[i]=readings[i];
  return l;
}



