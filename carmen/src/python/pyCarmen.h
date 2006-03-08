#include <carmen/carmen.h>
#include <cstdio>
#include <iostream>


class MessageHandler {
 private:
  carmen_robot_laser_message the_latest_msg_front;
  carmen_robot_laser_message the_latest_msg_rear;
  carmen_localize_globalpos_message the_latest_globalpos;
  carmen_base_odometry_message the_latest_odometry;
  carmen_base_sonar_message the_latest_sonar;
 public:
	virtual ~MessageHandler() 
	  { std::cout << "Callback::~Callback()" << std:: endl; }
	virtual void run(char* type, char* msg) //carmen_robot_laser_message *msg) 
	  { std::cout << "Callback::run()" << std::endl; }


	/*Front Laser Messages*/
	void set_front_laser_message(carmen_robot_laser_message *msg)
	  {memcpy(&the_latest_msg_front, msg, sizeof(carmen_robot_laser_message));}
	carmen_robot_laser_message* get_front_laser_message(void)
	  {return &the_latest_msg_front;}
	
	/*Rear Laser Messages*/
	void set_rear_laser_message(carmen_robot_laser_message *msg)
	  {memcpy(&the_latest_msg_rear, msg, sizeof(carmen_robot_laser_message));}
	carmen_robot_laser_message* get_rear_laser_message(void)
	  {return &the_latest_msg_rear;}

	/*Global Position Message*/
	void set_globalpos_message(carmen_localize_globalpos_message *msg)
	  {memcpy(&the_latest_globalpos, msg, sizeof(carmen_localize_globalpos_message));}
	carmen_localize_globalpos_message* get_globalpos_message()
	  {return &the_latest_globalpos;}

	/*Odometry Message*/
	void set_odometry_message(carmen_base_odometry_message *msg){
	  memcpy(&the_latest_odometry, msg, sizeof(carmen_base_odometry_message));
	}
	carmen_base_odometry_message* get_odometry_message() {return &the_latest_odometry;}

	/*Sonar Message*/
	void set_sonar_message(carmen_base_sonar_message *msg){
	  memcpy(&the_latest_sonar, msg, sizeof(carmen_base_sonar_message));
	}
	carmen_base_sonar_message* get_sonar_message() {return &the_latest_sonar;}
};



