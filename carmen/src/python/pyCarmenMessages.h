#include "pyCarmen.h"

static MessageHandler *_callback;

/*Deal with the front laser handler here*/
class front_laser{
 public:
  static void lazer_msg(carmen_robot_laser_message *msg)
  {
    _callback->set_front_laser_message(msg);
    if (_callback) _callback->run("front_laser", "get_front_laser_message()");
  }

  front_laser(MessageHandler *cb)
  {
    _callback = cb;
    carmen_robot_subscribe_frontlaser_message
      (NULL, (carmen_handler_t)lazer_msg, CARMEN_SUBSCRIBE_LATEST);
  }
};


/*Deal with the rear laser handler here*/
class rear_laser{
 public:
  static void lazer_msg_rear(carmen_robot_laser_message *msg)
  {
    _callback->set_rear_laser_message(msg);
    if (_callback) _callback->run("rear_laser", "get_rear_laser_message()");
  }

  rear_laser(MessageHandler *cb)
  {
    _callback = cb;
    carmen_robot_subscribe_rearlaser_message
      (NULL, (carmen_handler_t)lazer_msg_rear, CARMEN_SUBSCRIBE_LATEST);
  }
};


/*Deal with the rear laser handler here*/
class global_pose{
 public:

  static void my_callback(carmen_localize_globalpos_message *msg)
  {
    _callback->set_globalpos_message(msg);
    if (_callback) _callback->run("global_pose", "get_globalpos_message()");
  }

  global_pose(MessageHandler *cb)
  {    
    _callback = cb;
    carmen_localize_subscribe_globalpos_message
      (NULL, (carmen_handler_t)my_callback, CARMEN_SUBSCRIBE_LATEST);
  }
};

/*Deal with the odometry handler here*/
class odometry {
 public:

  static void my_callback(carmen_base_odometry_message *msg)
  {
    _callback->set_odometry_message(msg);
    if (_callback) _callback->run("odometry", "get_odometry_message()");
  }

  odometry(MessageHandler *cb)
  {    
    _callback = cb;
    carmen_base_subscribe_odometry_message
      (NULL, (carmen_handler_t)my_callback, CARMEN_SUBSCRIBE_LATEST);
  }
};


/*Deal with the odometry handler here*/
class sonar {
 public:

  static void my_callback(carmen_base_sonar_message *msg)
  {
    _callback->set_sonar_message(msg);
    if (_callback) _callback->run("sonar", "get_sonar_message()");
  }

  sonar(MessageHandler *cb)
  {    
    _callback = cb;
    carmen_base_subscribe_sonar_message
      (NULL, (carmen_handler_t)my_callback, CARMEN_SUBSCRIBE_LATEST);
  }
};



