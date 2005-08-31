package Carmen;

import java.util.*;
import IPC.*;
/** Carmen class for  a Robot */
public class Robot {
  private static final String CARMEN_ROBOT_SONAR_NAME = "robot_sonar";
  private static final String CARMEN_ROBOT_SONAR_FMT =
    "{int,double,<double:1>,<{double,double,double}:1>,{double,double,double},double,double,double,[char:10]}";

  private static final String CARMEN_ROBOT_FRONTLASER_NAME =
    "carmen_robot_frontlaser";
  private static final String CARMEN_ROBOT_REARLASER_NAME =
    "carmen_robot_rearlaser";

  private static final String CARMEN_ROBOT_LASER_FMT = 
    "{int,<float:1>,<char:1>,double,double,double,double,double,double,double,double,double,double,double,double,[char:10]}";

  private static final String CARMEN_ROBOT_VELOCITY_NAME = "carmen_robot_vel";
  private static final String CARMEN_ROBOT_VELOCITY_FMT = 
    "{double,double,double,[char:10]}";

  private static final String CARMEN_ROBOT_FOLLOW_TRAJECTORY_NAME =
    "carmen_robot_follow_trajectory";
  private static final String CARMEN_ROBOT_FOLLOW_TRAJECTORY_FMT = 
    "{int,<{double,double,double,double,double}:1>,{double,double,double,double,double},double,[char:10]}";

  private static final String CARMEN_ROBOT_VECTOR_MOVE_NAME = 
    "carmen_robot_vector_move";
  private static final String CARMEN_ROBOT_VECTOR_MOVE_FMT = 
    "{double,double,double,[char:10]}";

  private static final String CARMEN_ROBOT_BUMPER_NAME =
    "carmen_robot_bumper";
  private static final String CARMEN_ROBOT_BUMPER_FMT = 
    "{int, <char:1>, {double,double,double}, double, double, double, [char:10]}";

  private static final String CARMEN_ROBOT_CAMERA_NAME = 
    "carmen_robot_camera";
  private static final String CARMEN_ROBOT_CAMERA_FMT = "{int, int, int, int, <char:4>, double, double, double, double, double, double, double, double, double, [char:10]};";

  private static final String CARMEN_ROBOT_VECTOR_STATUS_NAME =
    "carmen_robot_vector_status";
  private static final String CARMEN_ROBOT_VECTOR_STATUS_FMT =
    "{double, double, double,[char:10]}";
  
  private static final String CARMEN_ROBOT_ODOMETRY_NAME = "carmen_base_odometry";
  private static final String CARMEN_ROBOT_ODOMETRY_FMT = 
  	"{double, double, double,double, double, double,double,[char:10]}";

  private static final String CARMEN_BASE_RESET_COMMAND_NAME = "carmen_base_reset_command";
  private static final String CARMEN_BASE_RESET_COMMAND_FMT = 
  	"{double,[char:10]}";

  /** Event Handler receives this and can set its fields */
  public static class CarmenBaseResetMessage {
    public double timestamp;
    public char host[];

  /** Event Handler receives this and can set its fields */
    CarmenBaseResetMessage() {
      timestamp = Util.getTimeMs();
      host = Util.getHostName();
    }
  }


/** Event Handler receives this and can set its fields */
  public static class RobotVelocityMessage {
    public double tv, rv;
    public double timestamp;
    public char host[];

  /** Event Handler receives this and can set its fields */
    RobotVelocityMessage(double tv, double rv) {
      this.tv = tv;
      this.rv = rv;
      timestamp = Util.getTimeMs();
      host = Util.getHostName();
    }
  }


  /** Event Handler receives this and can set its fields */
  public static class RobotVectorMoveMessage {
    public double distance, theta;
    public double timestamp;
    public char host[];

    RobotVectorMoveMessage(double distance, double theta) {
      this.distance = distance;
      this.theta = theta;
      timestamp = Util.getTimeMs();
      host = Util.getHostName();
    }
  }

  /** Event Handler receives this and can set its fields */
  public static class RobotFollowTrajectoryMessage {
    public int trajectory_length;
    public TrajPoint trajectory [];
    public TrajPoint robotPosition;
    public double timestamp;
    public char host[];
    
    RobotFollowTrajectoryMessage(TrajPoint trajectory [], TrajPoint
				 robotPosition) {
      this.trajectory_length = trajectory.length;
      this.trajectory = trajectory;
      this.robotPosition = robotPosition;
      timestamp = Util.getTimeMs();
      host = Util.getHostName();      
    }
  }

  private static class RobotPrivateSonarHandler implements IPC.HANDLER_TYPE {
    private static SonarHandler userHandler = null;
    RobotPrivateSonarHandler(SonarHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      SonarMessage message = (SonarMessage)callData;
      userHandler.handleSonar(message);
    }
  }

  private static class RobotPrivateFrontLaserHandler implements 
						       IPC.HANDLER_TYPE {
    private static FrontLaserHandler userHandler = null;
    RobotPrivateFrontLaserHandler(FrontLaserHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      LaserMessage message = (LaserMessage)callData;
      userHandler.handleFrontLaser(message);
    }
  }

  private static class RobotPrivateRearLaserHandler implements 
						       IPC.HANDLER_TYPE {
    private static RearLaserHandler userHandler = null;
    RobotPrivateRearLaserHandler(RearLaserHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      LaserMessage message = (LaserMessage)callData;
      userHandler.handleRearLaser(message);
    }
  }

  private static class RobotPrivateCameraHandler implements IPC.HANDLER_TYPE {
    private static CameraHandler userHandler = null;
    RobotPrivateCameraHandler(CameraHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      CameraMessage message = (CameraMessage)callData;
      userHandler.handleCamera(message);
    }
  }

  private static class RobotPrivateBumperHandler implements IPC.HANDLER_TYPE {
    private static BumperHandler userHandler = null;
    RobotPrivateBumperHandler(BumperHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      BumperMessage message = (BumperMessage)callData;
      userHandler.handleBumper(message);
    }
  }

  private static class RobotPrivateVectorStatusHandler implements IPC.HANDLER_TYPE {
    private static VectorStatusHandler userHandler = null;
    RobotPrivateVectorStatusHandler(VectorStatusHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      VectorStatusMessage message = (VectorStatusMessage)callData;
      userHandler.handleVectorStatus(message);
    }
  }
  
  private static class RobotPrivateOdometryHandler implements IPC.HANDLER_TYPE {
    private static OdometryHandler userHandler = null;
    RobotPrivateOdometryHandler(OdometryHandler userHandler) {
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
  public static void subscribeOdometry(OdometryHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_ODOMETRY_NAME, CARMEN_ROBOT_ODOMETRY_FMT);
    IPC.subscribeData(CARMEN_ROBOT_ODOMETRY_NAME, 
		      new RobotPrivateOdometryHandler(handler),
		      OdometryMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_ODOMETRY_NAME, 1);
  }
  
  /** Application module calls this to subscribe to SonarMessage.
   *  Application module must extend SonarHandler
   */
  public static void subscribeSonar(SonarHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_SONAR_NAME, CARMEN_ROBOT_SONAR_FMT);
    IPC.subscribeData(CARMEN_ROBOT_SONAR_NAME, 
		      new RobotPrivateSonarHandler(handler),
		      SonarMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_SONAR_NAME, 1);
  }

  /** Application module calls this to subscribe to LaserMessage.
   *  Application module must extend either FrontLaserHandler or RearLaserHandler.
   */
  public static void subscribeFrontLaser(FrontLaserHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_FRONTLASER_NAME, CARMEN_ROBOT_LASER_FMT);
    IPC.subscribeData(CARMEN_ROBOT_FRONTLASER_NAME, 
		      new RobotPrivateFrontLaserHandler(handler),
		      LaserMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_FRONTLASER_NAME, 1);
  }

  /** Application module calls this to subscribe to LaserMessage.
   *  Application module must extend either FrontLaserHandler or RearLaserHandler.
   */
  public static void subscribeRearLaser(RearLaserHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_REARLASER_NAME, CARMEN_ROBOT_LASER_FMT);
    IPC.subscribeData(CARMEN_ROBOT_REARLASER_NAME, 
		      new RobotPrivateRearLaserHandler(handler),
		      LaserMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_REARLASER_NAME, 1);
  }

  /** Application module calls this to subscribe to CameraMessage.
   *  Application module must extend CameraHandler
   */
  public static void subscribeCamera(CameraHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_CAMERA_NAME, CARMEN_ROBOT_CAMERA_FMT);
    IPC.subscribeData(CARMEN_ROBOT_CAMERA_NAME, 
		      new RobotPrivateCameraHandler(handler),
		      CameraMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_CAMERA_NAME, 1);
  }

  /** Application module calls this to subscribe to BumperMessage.
      Application module must extend BumperHandler.
   */
  public static void subscribeBumper(BumperHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_BUMPER_NAME, CARMEN_ROBOT_BUMPER_FMT);
    IPC.subscribeData(CARMEN_ROBOT_BUMPER_NAME, 
		      new RobotPrivateBumperHandler(handler),
		      BumperMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_BUMPER_NAME, 1);
  }

  /** Application module calls this to subscribe to VectorStatusMessage.
   *  Application module must extend VectorStatusHandler
   */
  public static void subscribeVectorStatus(VectorStatusHandler handler) {
    IPC.defineMsg(CARMEN_ROBOT_VECTOR_STATUS_NAME, 
		  CARMEN_ROBOT_VECTOR_STATUS_FMT);
    IPC.subscribeData(CARMEN_ROBOT_VECTOR_STATUS_NAME, 
		      new RobotPrivateVectorStatusHandler(handler),
		      VectorStatusMessage.class);
    IPC.setMsgQueueLength(CARMEN_ROBOT_VECTOR_STATUS_NAME, 1);
  }

  /** Application module calls this class method to direct the robot
   * at stated translational velocity (m/s) and rotational velocity (r/s).
   * This results in a message via IPC to the Orc_daemon then out to the 
   * robot base. To view how the message is processed in more detail, see
   * the Carmen c-code base in subdirectory orc_daemon file orclib.c and
   * procedure: carmen_base_direct_set_velocity 
   */
  public static void setVelocity(double tv, double rv) {
    RobotVelocityMessage velocityMessage = new RobotVelocityMessage(tv, rv);
    IPC.defineMsg(CARMEN_ROBOT_VELOCITY_NAME,
		  CARMEN_ROBOT_VELOCITY_FMT);
    IPC.publishData(CARMEN_ROBOT_VELOCITY_NAME, velocityMessage);
  }

  /** Application module calls this class method to direct the robot
   * to proceed in a vector stated by distance and orientation.
   */
   public static void moveAlongVector(double distance, double theta) {
    RobotVectorMoveMessage vectorMessage = 
      new RobotVectorMoveMessage(distance, theta);
    IPC.defineMsg(CARMEN_ROBOT_VECTOR_MOVE_NAME,
		  CARMEN_ROBOT_VECTOR_MOVE_FMT);
    IPC.publishData(CARMEN_ROBOT_VECTOR_MOVE_NAME, vectorMessage);
  }

/** Application module calls this class method to reset the 
   * robot base to the initialized state.
   */
   public static void resetRobotBase() {
    CarmenBaseResetMessage  resetMessage = 
      new CarmenBaseResetMessage();
    IPC.defineMsg(CARMEN_BASE_RESET_COMMAND_NAME,
		  CARMEN_BASE_RESET_COMMAND_FMT);
    IPC.publishData(CARMEN_BASE_RESET_COMMAND_NAME, resetMessage);
  }


   /** Application module calls this class method to direct the robot
   * to follow a trajectory given by a set of poses to a stated position.
   */
  public static void followTrajectory(TrajPoint trajectory[], 
				      TrajPoint robotPosition) {
    RobotFollowTrajectoryMessage trajectoryMessage = 
      new RobotFollowTrajectoryMessage(trajectory, robotPosition);
    IPC.defineMsg(CARMEN_ROBOT_FOLLOW_TRAJECTORY_NAME,
		  CARMEN_ROBOT_FOLLOW_TRAJECTORY_FMT);
    IPC.publishData(CARMEN_ROBOT_FOLLOW_TRAJECTORY_NAME, trajectoryMessage);
  }

  public static void initialize(String moduleName, String SBCName)
  {
    IPC.connectModule(moduleName, SBCName);
  }

  public static void dispatch()
  {
    IPC.dispatch();
  }

  // Dispatch but only for as long as timeoutMSecs 

  public static int listen (long timeoutMSecs) {
    return IPC.listen(timeoutMSecs);
  }

}
