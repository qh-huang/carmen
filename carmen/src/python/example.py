from 

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
