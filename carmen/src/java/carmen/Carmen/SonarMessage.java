package Carmen;
/**  Carmen SonarHandler's message
   */

public class SonarMessage {
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
  public double timestamp;
  public char host[];
}

