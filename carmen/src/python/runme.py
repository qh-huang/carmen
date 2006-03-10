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


def test1():
	# Create an callback
	cb = pyMessageHandler().__disown__()
	
	# Create callers 
	fl = pyCarmen.front_laser(cb)
	rl = pyCarmen.rear_laser(cb)
	loc = pyCarmen.global_pose(cb)
	odo = pyCarmen.odometry(cb)
	son = pyCarmen.sonar(cb)
	bum  = pyCarmen.bumper(cb)
	nav_st = pyCarmen.navigator_status(cb)
	nav_pl = pyCarmen.navigator_plan(cb)
	nav_stop = pyCarmen.navigator_stopped(cb)
	
	# Dispatch and don't return
	cb.connect()

def test2():
	cb = Robot().__disown__()
	
	loc = pyCarmen.global_pose(cb)

	cb.set_pose(30, 5, 0)
	cb.start()
	
	while(1):
		#cb.velocity(5, 5)
		#cb.vector(5, 5)
		#cb.set_goal(14.5,3.8)
		#cb.go()

		cb.get_gridmap()
		if(raw_input() == "q"):
			break
		

	cb.disconnect()
	print "done"


	
#test1()
test2()
#test3()

