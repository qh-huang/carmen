from pyRobot import *
from math import *

def test1():
	# Create an callback
	robot = pyMessageHandler().__disown__()
	# Create callers
        # the exact syntax of these might change a bit
	fl = pyCarmen.front_laser(robot)
	rl = pyCarmen.rear_laser(robot)
	loc = pyCarmen.global_pose(robot)
	odo = pyCarmen.odometry(robot)
	son = pyCarmen.sonar(robot)
	bum  = pyCarmen.bumper(robot)
	nav_st = pyCarmen.navigator_status(robot)
	nav_pl = pyCarmen.navigator_plan(robot)
	nav_stop = pyCarmen.navigator_stopped(robot)
	the_arm = pyCarmen.arm(robot)
	
	# Dispatch and don't return
	robot.connect()

def test2():
	robot = Robot().__disown__()
	#loc = pyCarmen.global_pose(robot)
	odo = pyCarmen.odometry(robot)

	#robot.set_pose(30, 5, 0)
	robot.start()
	while(1):
                try:
                    #set the tv and rv of the robot
  		    #robot.velocity(5, 5)

                    #set the vector of the direction of the robot
  		    #robot.vector(5, 5)

                    #set the goal of the robot and go there
		    #robot.set_goal(20.5,3.8)
                    #robot.go()

		    #test the arm
		    #make sure messages are being recieved from the arm
		    #robot.command_arm([pi/4.0, pi/4, pi/4])
		    
		    mycmd = raw_input(">> ")
		    eval(mycmd)

                    #not quite functioning yet is getting a gridmap
                    #of the world
		    #robot.get_gridmap()
                    

		except(KeyboardInterrupt):
                    print "exiting..."
                    break

	        except:
		    print "Command not found..."

	robot.stop()
	print "done"


	
#test1()
test2()
