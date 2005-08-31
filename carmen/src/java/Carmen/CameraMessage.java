package Carmen;
/**  Carmen CameraHandler's message
   */

public class CameraMessage {
  /** unit is pixels */
  public int width;
  /** unit is pixels */
  public int height;
  public int bytes_per_char;
  /** size of image[]  */
  public int image_size;
  /** camera image */
  public char image[];
  /** position of the camera on the robot */
  public double x, y, theta;
 /** robot pose at the center of the robot */
  public double odom_x, odom_y, odom_theta; 
  /** commanded translational velocity (m/s) */
  public double tv;
  /** commanded rotational velocity (r/s) */
  public double rv;
  /** Timestamp from host locally running orcd daemon */
  public double timestamp;
  public char host[];
}

