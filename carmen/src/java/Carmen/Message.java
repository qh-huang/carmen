package Carmen;
import IPC.*;

/** Carmen generic or base Message class */

public class Message {
  public double timestamp;
  public char host[];  

  /** New message has timestamp from this host and host name */
  Message() {
    timestamp = Util.getTimeMs();
    host = Util.getHostName();
  }


}

