import Carmen.*;

public class TestRobot implements FrontLaserHandler, OdometryHandler {
  private boolean initialized = false;
  private double startTime = 0;

  public void handleOdometry (OdometryMessage message) {
    if (!initialized) {
      startTime = message.timestamp;
      initialized = true;
      return;
    }
    message.timestamp -= startTime;
    System.out.println("Odometry: "+message.tv+" m/s "+
		       Util.radsToDegrees(message.rv)+" deg/s");
    if (message.timestamp > 2) {
      System.exit(0);
    }
  }

  public void handleFrontLaser (LaserMessage message) {
    if (!initialized) {
      startTime = message.timestamp;
      initialized = true;
      return;
    }
    message.timestamp -= startTime;
    System.out.println("LaserHandler: Got "+message.num_readings);
    if (message.timestamp > 2) {
      System.exit(0);
    }
  }

  public static void main (String args[]) {
    Robot.initialize("TestRobot", "localhost");
    TestRobot test = new TestRobot();
    Robot.subscribeOdometry(test);
    Robot.subscribeFrontLaser(test);
    Robot.setVelocity(0, Util.degreesToRads(30));
    Robot.dispatch();
  }
}
