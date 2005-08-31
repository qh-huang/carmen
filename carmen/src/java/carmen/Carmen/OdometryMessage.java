package Carmen;
/**  Carmen OdometryHandler's message
   */

public class OdometryMessage {
  /** robot pose at the center of the robot */
  public double x, y, theta;
  /** commanded translational (m/s) and rotational (r/s) velocity */
  public double tv, rv;
  /** robot accelateration (m/s^2) */
  public double acceleration;
  /** Timestamp from host locally running orc daemon */
  public double timestamp;
  public char host[];
}

