package Carmen;

import IPC.*;

/**  Carmen OdometryHandler's message
   */

public class OdometryMessage extends Message {
  /** robot pose at the center of the robot */
  public double x, y, theta;
  /** commanded translational (m/s) and rotational (r/s) velocity */
  public double tv, rv;
  /** robot acceleration (m/s^2) */
  public double acceleration;

  private static final String CARMEN_ROBOT_ODOMETRY_NAME = "carmen_base_odometry";
  private static final String CARMEN_ROBOT_ODOMETRY_FMT = 
  	"{double, string, double, double, double, double, double, double}";

  private static class PrivateOdometryHandler implements IPC.HANDLER_TYPE {
    private static OdometryHandler userHandler = null;
    PrivateOdometryHandler(OdometryHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      OdometryMessage message = (OdometryMessage)callData;
      userHandler.handleOdometry(message);
    }
  }

  /** Application module calls this to subscribe to OdometryMessage.
   *  Application module must extend OdometryHandler.
   */
  public static void subscribe(OdometryHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_ODOMETRY_NAME, CARMEN_ROBOT_ODOMETRY_FMT);
    IPC.subscribeData(CARMEN_ROBOT_ODOMETRY_NAME, 
		      new PrivateOdometryHandler(handler),
		      OdometryMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_ODOMETRY_NAME, 1);
  }
  
}

