package Carmen;

/**  Carmen BumperHandler's message
   */
public class BumperMessage {
  
  public int num_bumpers; 
  /** state of bumper */
  public char bumper[]; 
  /** robot pose at the center of the robot */
  public Point robot_location; 
  /** commanded translational velocity (m/s) */
  public double tv;
  /** commanded rotational velocity (r/s) */
  public double rv;
  /** Timestamp from host locally running orcd daemon */
  public double timestamp;
  public char host[];
}

