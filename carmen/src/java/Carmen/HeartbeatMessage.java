package Carmen;

import IPC.*;

/**  Carmen HeartbeatHandler's message
   */

public class HeartbeatMessage extends Message {
  public String module;
  public int pid;

  private static final String CARMEN_HEARTBEAT_NAME =
    "carmen_heartbeat";
  private static final String CARMEN_HEARTBEAT_FMT =
    "{string, int, double, string}";

  private static boolean defined = false;

  private static class PrivateHeartbeatHandler implements IPC.HANDLER_TYPE {
    private static HeartbeatHandler userHandler = null;
    PrivateHeartbeatHandler(HeartbeatHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      HeartbeatMessage message = (HeartbeatMessage)callData;
      userHandler.handleHeartbeat(message);
    }
  }

  public static void subscribe(HeartbeatHandler handler)
  {
    if (!defined) {
      IPC.defineMsg(CARMEN_HEARTBEAT_NAME, CARMEN_HEARTBEAT_FMT);
      defined = true;
    }
    IPC.subscribeData(CARMEN_HEARTBEAT_NAME,
                      new PrivateHeartbeatHandler(handler),
                      HeartbeatMessage.class);
  }

  public void publish() {
    if (!defined) {
      IPC.defineMsg(CARMEN_HEARTBEAT_NAME, CARMEN_HEARTBEAT_FMT);
      defined = true;
    }
    IPC.publishData(CARMEN_HEARTBEAT_NAME, this);
  }

  public static void publish(String module)
  {
    HeartbeatMessage msg = new HeartbeatMessage();
    msg.module = module;
    msg.pid = (int)IPC.getPID();
    msg.publish();
  }

}

