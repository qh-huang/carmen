# Create an callback
robot = pyMessageHandler().__disown__()
# Create callers
# the exact syntax of these might change a bit
laser = pyCarmen.laser5(robot)

# Dispatch and don't return
robot.connect()