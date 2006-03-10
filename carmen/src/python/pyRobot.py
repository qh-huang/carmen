# file: runme.py
# This file illustrates the cross language polymorphism using directors.

import pyCarmen
from threading import Thread
from time import sleep
from sys import exit
from scipy import *

#You need to override the callback function in order
#to get the functionality of this class
class pyMessageHandler(pyCarmen.MessageHandler, Thread):
	def __init__(self):
		pyCarmen.carmen_ipc_initialize(1, ["python"])
		pyCarmen.MessageHandler.__init__(self)
		Thread.__init__(self)
		self.initialize()
		self.laser = None
	def run_cb(self, my_type, msg_functor):
		msg = eval("self."+msg_functor)
		self.callback(my_type, msg)

	def callback(self, the_type, msg):
		#print the_type
		#print dir(msg)
		pass

	def run(self):
		self.connect()

	def __del__(self):
		print "PyCallback.__del__()"
		pyCarmen.MessageHandler.__del__(self)

	def connect(self):
		pyCarmen.carmen_ipc_dispatch()

	def disconnect(self):
		pyCarmen.carmen_ipc_disconnect()


class Robot(pyMessageHandler):
	def __init__(self):
		pyMessageHandler.__init__(self)

	def velocity(self, tv, rv):
		pyCarmen.carmen_robot_velocity_command(tv, rv)

	def vector(self, distance, theta):
		pyCarmen.carmen_robot_move_along_vector(distance, theta)

	def callback(self, the_type, msg):
		#print the_type
		
		#if(the_type == "global_pose"):
		#     print "global_pose", msg.globalpos.x, msg.globalpos.y
		pass

	def set_pose(self, x, y, theta):
		pt = pyCarmen.carmen_point_t()
		pt.x = x
		pt.y = y
		pt.theta = theta
		pyCarmen.carmen_simulator_set_truepose(pt)

	def set_goal(self, x, y):
		pyCarmen.carmen_navigator_set_goal(x, y)

	def set_goal_place(self, name):
		pyCarmen.carmen_navigator_set_goal_place(name)

	def go(self):
		pyCarmen.carmen_navigator_go()

	def stop(self):
		pyCarmen.carmen_navigator_stop()

	def get_gridmap(self):
		theMap = zeros([self.get_map_x_size(), self.get_map_y_size()])
		
		print "starting"
		print "size", [self.get_map_x_size(), self.get_map_y_size()]
		for i in range(self.get_map_x_size()):
			print i
			for j in range(self.get_map_y_size()):
				print self.get_map_x_y(i,j)
				theMap[i][j] = self.get_map_x_y(i,j)

		print "done"
		return theMap
		#pyCarmen.carmen_map_get_gridmap_t(pyCarmen.carmen_map_tPtr(my_current_map))
		#print dir(my_current_map)
		#print my_current_map.complete_map
		#print my_current_map.map
		#carmen_map_get_gridmap(pyCarmen.carmen_map_tPtr(my_current_map))




