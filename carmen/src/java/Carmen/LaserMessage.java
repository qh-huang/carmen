package Carmen;
/**  Carmen FrontLaserHandler's and RearLaserHandler's message
   */

public class LaserMessage {
  public int num_readings;
  public float range[];
  public char tooclose[];
  /** position of the laser on the robot */
  public double x, y, theta;
  /** position of the center of the robot  */
  public double odom_x, odom_y, odom_theta; 
  /** robot state: translational velocity and rotational velocity */
  public double tv, rv;
  /** application defined safety distance in metres */
  public double forward_safety_dist, side_safety_dist;
  public double turn_axis;
  public double timestamp;
  public char host[];
}

