# file: runme.py

# This file illustrates the cross language polymorphism using directors.
import pyCarmen

#You need to override the callback function in order
#to get the functionality of this class
class pyMessageHandler(pyCarmen.MessageHandler):
	def __init__(self):
		pyCarmen.carmen_ipc_initialize(1, ["python"])
		pyCarmen.MessageHandler.__init__(self)
		self.laser = None
	def run(self, my_type, msg_functor):
		msg = eval("self."+msg_functor)
		self.callback(my_type, msg)

	def callback(self, the_type, msg):
		print the_type

		#if(the_type == "odometry"):
		#	print dir(msg)
		
		#if(the_type == "global_pose"):
		#	print dir(msg)
		#	print msg.globalpos_xy_cov
		
	def __del__(self):
		print "PyCallback.__del__()"
		pyCarmen.MessageHandler.__del__(self)

	def dispatch(self):
		pyCarmen.carmen_ipc_dispatch()



# Create an callback
cb = pyMessageHandler().__disown__()

# Create callers 
fl = pyCarmen.front_laser(cb)
rl = pyCarmen.rear_laser(cb)
loc = pyCarmen.global_pose(cb)
odo = pyCarmen.odometry(cb)
son = pyCarmen.sonar(cb)

# Dispatch and don't return
cb.dispatch()


