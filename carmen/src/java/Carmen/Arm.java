package Carmen;

import IPC.*;

public class Arm {
  private static final String CARMEN_BASE_SERVO_ARM_NAME = "carmen_base_servo_arm_command";
  private static final String CARMEN_BASE_SERVO_ARM_QUERY_NAME = "carmen_base_servo_arm_query";
  private static final String CARMEN_BASE_SERVO_ARM_FMT = 
    "{<double:2>, int, double, [char:10]}";

  public static class ServoArmMessage {
    public double servos [];
    public int numServos;
    public double timestamp;
    public char host[];

    ServoArmMessage(double servos[]) {
      this.servos = servos;
      numServos=servos.length;
      timestamp = Util.getTimeMs();
      host = Util.getHostName();
    }
  }    

  public static void servo(double servoPositions[])
  {
    if (servoPositions.length > 4) {
      System.out.println("Arm has only 4 servos. You called Arm.servo with "+
			 servoPositions.length+" ints. Ignoring.");
      return;
    }
    ServoArmMessage servoMessage =
      new ServoArmMessage(servoPositions);
    IPC.defineMsg(CARMEN_BASE_SERVO_ARM_NAME, CARMEN_BASE_SERVO_ARM_FMT);
    IPC.publishData(CARMEN_BASE_SERVO_ARM_NAME, servoMessage);
  }

  public static double [] query()
  {
    Message msg = new Message();
    ServoArmMessage response = (ServoArmMessage)IPC.queryResponseData
      (CARMEN_BASE_SERVO_ARM_QUERY_NAME, msg, ServoArmMessage.class, 5000);
    return response.servos;
  }
}
