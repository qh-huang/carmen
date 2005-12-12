package Carmen;

import IPC.*;

/**  Carmen BumperHandler's message
   */
public class BumperMessage extends Message {
  
  public int num_bumpers; 
  /** state of bumper */
  public char bumper[]; 
  /** robot pose at the center of the robot */
  public Point robot_location; 
  /** commanded translational velocity (m/s) */
  public double tv;
  /** commanded rotational velocity (r/s) */
  public double rv;

  private static final String CARMEN_ROBOT_BUMPER_NAME =
    "carmen_robot_bumper";
  private static final String CARMEN_ROBOT_BUMPER_FMT = 
    "{double, string, int, <char:3>, {double,double,double}, double, double}";

  private static class PrivateBumperHandler implements IPC.HANDLER_TYPE {
    private static BumperHandler userHandler = null;
    PrivateBumperHandler(BumperHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      BumperMessage message = (BumperMessage)callData;
      userHandler.handleBumper(message);
    }
  }

  /** Application module calls this to subscribe to BumperMessage.
      Application module must extend BumperHandler.
   */
  public static void subscribe(BumperHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_BUMPER_NAME, CARMEN_ROBOT_BUMPER_FMT);
    IPC.subscribeData(CARMEN_ROBOT_BUMPER_NAME, 
		      new PrivateBumperHandler(handler),
		      BumperMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_BUMPER_NAME, 1);
  }

}

