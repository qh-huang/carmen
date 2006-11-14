#include "laser_messages.h"
#include <carmen/carmen.h>
#include <assert.h>

static char* msgnames[10]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

char* carmen_laser_get_messagename(int laser_id) {
  //   assert(laser_id>= && laser_id<=100);
   if (msgnames[laser_id])
	return msgnames[laser_id];
   char name_result[1024];
   sprintf(name_result, "carmen_laser_laser%d", laser_id);
   msgnames[laser_id]=malloc(strlen(name_result)+1);
   strcpy(msgnames[laser_id],name_result);
   return msgnames[laser_id];
}

void
carmen_laser_subscribe_laser_message(int laser_id,
				     carmen_laser_laser_message *laser,
				     carmen_handler_t handler,
				     carmen_subscribe_t subscribe_how)
{
  char* msg_name=carmen_laser_get_messagename(laser_id);
  //carmen_warn("subscribing: %x,%s\n", (unsigned int)msg_name,  msg_name);
  carmen_subscribe_message(msg_name,
			   CARMEN_LASER_LASER_FMT,
			   laser, sizeof(carmen_laser_laser_message), handler,
			   subscribe_how);
}

void
carmen_laser_unsubscribe_laser_message(int laser_id,
				       carmen_handler_t handler)
{
  char* msg_name=carmen_laser_get_messagename(laser_id);
  carmen_unsubscribe_message(msg_name, 
			     handler);
}


void
carmen_laser_define_laser_message(int laser_id)
{
  char* msg_name=carmen_laser_get_messagename(laser_id);
  //  carmen_warn("defining: %x,%s\n", (unsigned int)msg_name,  msg_name);
  IPC_RETURN_TYPE err;
  
  err = IPC_defineMsg(msg_name,IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_LASER_FMT);
  carmen_test_ipc_exit(err, "Could not define", msg_name);

//  carmen_ipc_define_test_exit(msg_name, CARMEN_LASER_LASER_FMT);
}


void
carmen_laser_publish_laser_message(int laser_id, carmen_laser_laser_message* msg) {

  char* msg_name=carmen_laser_get_messagename(laser_id);

  if (!msg)
    return;

  //  carmen_warn("sending: %x,%s\n", (unsigned int)msg_name,  msg_name);
  IPC_RETURN_TYPE err;
  err = IPC_publishData(msg_name, msg);
  carmen_test_ipc_exit(err, "Could not publish", 
			 msg_name);
}


void
carmen_laser_subscribe_alive_message(carmen_laser_alive_message *alive,
				     carmen_handler_t handler,
				     carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_ALIVE_NAME, 
			   CARMEN_LASER_ALIVE_FMT,
			   alive, sizeof(carmen_laser_alive_message), handler,
			   subscribe_how);
}

void
carmen_laser_unsubscribe_alive_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_ALIVE_NAME, handler);
}

void
carmen_laser_define_alive_message()
{
  carmen_ipc_define_test_exit(CARMEN_LASER_ALIVE_NAME, 
			      CARMEN_LASER_ALIVE_FMT);
}


/* void */
/* carmen_laser_publish_alive_message(carmen_laser_alive_message* msg) { */

/*   if (!msg) */
/*     return; */
/*   carmen_ipc_publish_exit(CARMEN_LASER_ALIVE_NAME, msg); */
/* } */




void
carmen_laser_subscribe_laser1_message(carmen_laser_laser_message *laser,
				     carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how) {

  carmen_laser_subscribe_laser_message(1, laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_laser2_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how) {

  carmen_laser_subscribe_laser_message(2, laser, handler, subscribe_how);
}


void
carmen_laser_subscribe_laser3_message(carmen_laser_laser_message *laser,
				     carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how){

  carmen_laser_subscribe_laser_message(3, laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_laser4_message(carmen_laser_laser_message *laser,
				     carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how) {

  carmen_laser_subscribe_laser_message(4, laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_laser5_message(carmen_laser_laser_message *laser,
				     carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how) {

  carmen_laser_subscribe_laser_message(5, laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_frontlaser_message(carmen_laser_laser_message *laser,
					  carmen_handler_t handler,
					  carmen_subscribe_t subscribe_how){
  carmen_laser_subscribe_laser1_message(laser, handler, subscribe_how);
  
}

void
carmen_laser_subscribe_rearlaser_message(carmen_laser_laser_message *laser,
				     carmen_handler_t handler,
					 carmen_subscribe_t subscribe_how){
  carmen_laser_subscribe_laser2_message(laser, handler, subscribe_how);

}
