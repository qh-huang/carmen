package Carmen;

import IPC.*;
 
public class ArmMessage {
  public double servos[];
  public int numServos;
  public double servoCurrents[];
  public int numCurrents;
  public int gripperState;
  public double timestamp;
  public char host[];

  
  private static final String CARMEN_BASE_ARM_STATE_NAME = 
    "carmen_base_arm_state";
  private static final String CARMEN_BASE_ARM_STATE_FMT =  
    "{<double:2>, int, <double:4>, int, int, double, [char:10]}";

  private static class PrivateArmHandler implements IPC.HANDLER_TYPE {
    private static ArmHandler userHandler = null;
    PrivateArmHandler(ArmHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      ArmMessage message = (ArmMessage)callData;
      userHandler.handleArm(message);
    }
  }

  public static void subscribe(ArmHandler handler)
  {
    IPC.defineMsg(CARMEN_BASE_ARM_STATE_NAME,
                  CARMEN_BASE_ARM_STATE_FMT);
    IPC.subscribeData(CARMEN_BASE_ARM_STATE_NAME,
                      new PrivateArmHandler(handler),
                      ArmMessage.class);
    IPC.setMsgQueueLength(CARMEN_BASE_ARM_STATE_NAME, 1);
  }

}
