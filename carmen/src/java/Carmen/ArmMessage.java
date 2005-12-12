package Carmen;

import IPC.*;
 
public class ArmMessage extends Message {
  public double servos[];
  public int numServos;
  public double servoCurrents[];
  public int numCurrents;
  public int gripperState;
  
  private static final String CARMEN_BASE_ARM_STATE_NAME = 
    "carmen_base_arm_state";
  private static final String CARMEN_BASE_ARM_STATE_FMT =  
    "{double, string, <double:4>, int, <double:6>, int, int}";
  private static boolean defined = false;

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
    if (!defined) {
      IPC.defineMsg(CARMEN_BASE_ARM_STATE_NAME, CARMEN_BASE_ARM_STATE_FMT);
      defined = true;
    }

    IPC.subscribeData(CARMEN_BASE_ARM_STATE_NAME,
                      new PrivateArmHandler(handler),
                      ArmMessage.class);
    IPC.setMsgQueueLength(CARMEN_BASE_ARM_STATE_NAME, 1);
  }

}
