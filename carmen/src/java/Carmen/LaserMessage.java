package Carmen;

import IPC.*;

/**  Carmen FrontLaserHandler's and RearLaserHandler's message
   */

public class LaserMessage extends Message {
  public int num_readings;
  public float range[];
  public char tooclose[];
  /** position of the laser on the robot */
  public double x, y, theta;
  /** position of the center of the robot  */
  public double odom_x, odom_y, odom_theta; 
  /** robot state: translational velocity and rotational velocity */
  public double tv, rv;
  /** application defined safety distance in metres */
  public double forward_safety_dist, side_safety_dist;
  public double turn_axis;

  private static final String CARMEN_ROBOT_FRONTLASER_NAME =
    "carmen_robot_frontlaser";
  private static final String CARMEN_ROBOT_REARLASER_NAME =
    "carmen_robot_rearlaser";

  private static final String CARMEN_ROBOT_LASER_FMT = 
    "{double,string,int,<float:1>,<char:1>,double,double,double,double,double,double,double,double,double,double,double}";

  private static class PrivateFrontLaserHandler implements 
						       IPC.HANDLER_TYPE {
    private static FrontLaserHandler userHandler = null;
    PrivateFrontLaserHandler(FrontLaserHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      LaserMessage message = (LaserMessage)callData;
      userHandler.handleFrontLaser(message);
    }
  }

  private static class PrivateRearLaserHandler implements 
						       IPC.HANDLER_TYPE {
    private static RearLaserHandler userHandler = null;
    PrivateRearLaserHandler(RearLaserHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      LaserMessage message = (LaserMessage)callData;
      userHandler.handleRearLaser(message);
    }
  }

  /** Application module calls this to subscribe to LaserMessage.
   *  Application module must extend either FrontLaserHandler or RearLaserHandler.
   */
  public static void subscribeFront(FrontLaserHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_FRONTLASER_NAME, CARMEN_ROBOT_LASER_FMT);
    IPC.subscribeData(CARMEN_ROBOT_FRONTLASER_NAME, 
		      new PrivateFrontLaserHandler(handler),
		      LaserMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_FRONTLASER_NAME, 1);
  }

  /** Application module calls this to subscribe to LaserMessage.
   *  Application module must extend either FrontLaserHandler or RearLaserHandler.
   */
  public static void subscribeRear(RearLaserHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_REARLASER_NAME, CARMEN_ROBOT_LASER_FMT);
    IPC.subscribeData(CARMEN_ROBOT_REARLASER_NAME, 
		      new PrivateRearLaserHandler(handler),
		      LaserMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_REARLASER_NAME, 1);
  }

}

