import pyCarmen
from pylab import *
from pyRobot import pyMessageHandler
from scipy import *
from sys import argv


ion()
X, Y = [], []
laser_plot, = plot(X, Y)

class myRobot(pyMessageHandler):
    def __init__(self):
        print "starting"
        pyMessageHandler.__init__(self)

    def callback(self, the_type, message):
        global X, Y

        print "tk:", the_type
        print "laser config", message["laser_config"]
        start_angle = message["laser_config"]["start_angle"]
        increment = message["laser_config"]["angular_resolution"]

        #print message["laser_config"]["start_angle"]
        th = arange(0.001,increment*len(message["range"]), increment)+(pi/180.0)*message["laser_config"]["start_angle"]
        
        X,Y = [],[]
        i = 0
        R = array(message["range"])
        
        for t in th:
            X.append(R[i]*cos(t))
            Y.append(R[i]*sin(t))
            i+=1


        laser_plot.set_data(X, Y)
        max_range = message["laser_config"]["maximum_range"]
        axis([-max_range-1, max_range+1, -max_range-1, max_range+1])
        draw()



if __name__ == "__main__":
    

    # Create an callback
    robot = myRobot().__disown__()
    
    # Create callers
    # the exact syntax of these might change a bit
    if(not len(argv) == 2):
        print "usage: python viewlaser {frontlaser, rearlaser, laser5}"

    if(argv[1] == "frontlaser"):
        fl = pyCarmen.front_laser(robot)

    elif(argv[1] == "rearlaser"):
        rl = pyCarmen.rear_laser(robot)
    
    elif(argv[1] == "laser5"):
        laser = pyCarmen.laser5(robot)
    
    # Dispatch and don't return
    robot.connect()
