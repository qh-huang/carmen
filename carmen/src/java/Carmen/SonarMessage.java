package Carmen;

import IPC.*;

/**  Carmen SonarHandler's message
   */

public class SonarMessage extends Message {
  public int num_sonars;
  /** width of sonar cone */
  public double sensor_angle;   
  /** for each sonar, the range reading */
  public double range[];
  /** location of each sonar with respect to robot as a Point */
  public Point sonar_positions[];
  /** robot state: point location */
  public Point robot_location;
  public double tv;
  public double rv;

  private static final String CARMEN_ROBOT_SONAR_NAME = "robot_sonar";
  private static final String CARMEN_ROBOT_SONAR_FMT =
    "{double,string,int,double,<double:3>,<{double,double,double}:3>,{double,double,double},double,double}";

  private static class PrivateSonarHandler implements IPC.HANDLER_TYPE {
    private static SonarHandler userHandler = null;
    PrivateSonarHandler(SonarHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      SonarMessage message = (SonarMessage)callData;
      userHandler.handleSonar(message);
    }
  }

  /** Application module calls this to subscribe to SonarMessage.
   *  Application module must extend SonarHandler
   */
  public static void subscribe(SonarHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_SONAR_NAME, CARMEN_ROBOT_SONAR_FMT);
    IPC.subscribeData(CARMEN_ROBOT_SONAR_NAME, 
		      new PrivateSonarHandler(handler),
		      SonarMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_SONAR_NAME, 1);
  }

}

