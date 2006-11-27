#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <carmen/carmen.h>
#include "carmen_hokuyo.h"
#include "hokuyourg.h"
#include <values.h>
#include "laser_messages.h"

int carmen_hokuyo_init(carmen_laser_device_t* device){
  HokuyoURG* urg=malloc(sizeof(HokuyoURG));
  carmen_test_alloc(urg);
  device->device_data=urg;
  hokuyo_init(urg);
  return 1;
}

#define URG_VERSION_BUFSIZE 2048
int carmen_hokuyo_connect(carmen_laser_device_t * device, char* filename, int baudrate __attribute__((unused)) ){
  int result;
  char buf[URG_VERSION_BUFSIZE];
  HokuyoURG* urg=(HokuyoURG*)device->device_data;
  result=hokuyo_open(urg,filename);
  if (! result){
    fprintf(stderr, "error\n  Unable to opening device\n");
    return result;
  }
  fprintf(stderr, "\nQuerying version from device:\n");
  result = hokuyo_getVersion(urg, buf, -1);
  if (result==-1){
    fprintf(stderr, "  Error in querying version file\n");
    return result;
  }
  printf("  Version is %s\n",buf);
  return 1;
}

int carmen_hokuyo_configure(carmen_laser_device_t * device ){
  HokuyoURG* urg=(HokuyoURG*)device->device_data;
  device->config.start_angle=hokuyo_getStartAngle(urg,-1);
  device->config.angular_resolution=hokuyo_getStep(urg,-1);
  device->config.fov=device->config.angular_resolution*(urg->endStep-urg->startStep);
  device->config.accuracy=0.001;
  device->config.maximum_range=4.095;	
  return 1;
}

int carmen_hokuyo_handle_sleep(carmen_laser_device_t* device __attribute__ ((unused)) ){
  sleep(1);
  return 1;
}

int carmen_hokuyo_handle(carmen_laser_device_t* device){
  HokuyoURG* urg=(HokuyoURG*)device->device_data;
  unsigned short readings[1024];
  struct timeval timestamp;
  int size=hokuyo_getReading(urg, readings, &timestamp, -1, -1, -1, -1, -1);
  if (size){
    int j;
    carmen_laser_laser_static_message message;
    message.id=device->laser_id;
    message.config=device->config;
    message.num_readings=size;
    message.num_remissions=0;
    message.timestamp=(double)timestamp.tv_sec+1e-6*timestamp.tv_usec;
    for (j=0; j<size; j++){
      message.range[j]=0.001*readings[j];
    }
    if (device->f_onreceive!=NULL)
      (*device->f_onreceive)(device, &message);
    return 1;
  }
  return 0;
}

int carmen_hokuyo_start(carmen_laser_device_t* device){
  HokuyoURG* urg=(HokuyoURG*)device->device_data;
  fprintf(stderr, "Switching laser on ................ "); 	
  int retVal=hokuyo_laserOn(urg,-1);
  if(retVal)
    fprintf(stderr, "success\n");
  else {
    fprintf(stderr, "error\n");
    return 0;
  }
  device->f_handle=carmen_hokuyo_handle;
  return 1;
}

int carmen_hokuyo_stop(carmen_laser_device_t* device){
  HokuyoURG* urg=(HokuyoURG*)device->device_data;
  fprintf(stderr, "Switching laser off ............... "); 	
  int retVal=hokuyo_laserOff(urg,-1);
  if(retVal)
    fprintf(stderr, "success\n");
  else {
    fprintf(stderr, "error\n");
    return 0;;
  }
  device->f_handle=carmen_hokuyo_handle_sleep;
  return 1;
}



//FIXME I do not want  to malloc the ranges!

int carmen_hokuyo_close(struct carmen_laser_device_t* device){
  HokuyoURG* urg=(HokuyoURG*)device->device_data;
  return hokuyo_close(urg);
}

carmen_laser_device_t* carmen_create_hokuyo_instance(carmen_laser_laser_config_t* config, int laser_id){
  fprintf(stderr,"init hokuyo\n");
  carmen_laser_device_t* device=(carmen_laser_device_t*)malloc(sizeof(carmen_laser_device_t));
  carmen_test_alloc(device);
  device->laser_id=laser_id;
  device->config=*config;
  device->f_init=carmen_hokuyo_init;
  device->f_connect=carmen_hokuyo_connect;
  device->f_configure=carmen_hokuyo_configure;
  device->f_start=carmen_hokuyo_start;
  device->f_stop=carmen_hokuyo_stop;
  device->f_handle=carmen_hokuyo_handle_sleep;
  device->f_close=carmen_hokuyo_close;
  device->f_onreceive=NULL;
  return device;
}


carmen_laser_laser_config_t carmen_hokuyo_valid_configs[]=
  {{HOKUYO_URG,MAXDOUBLE,MAXDOUBLE,MAXDOUBLE,MAXDOUBLE,MAXDOUBLE,REMISSION_NONE}};

int carmen_hokuyo_valid_configs_size=1;

int carmen_init_hokuyo_configs(void){
  int i;
  carmen_laser_laser_config_t* conf=carmen_laser_configurations+carmen_laser_configurations_num;
  for (i=0; i<carmen_hokuyo_valid_configs_size; i++){
    *conf=carmen_hokuyo_valid_configs[i];
    conf++;
  }
  carmen_laser_configurations_num+=carmen_hokuyo_valid_configs_size;
  return carmen_laser_configurations_num;
}



