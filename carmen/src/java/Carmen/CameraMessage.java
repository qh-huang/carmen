package Carmen;

import IPC.*;

/**  Carmen CameraHandler's message
   */

public class CameraMessage extends Message {
  /** unit is pixels */
  public int width;
  /** unit is pixels */
  public int height;
  public int bytes_per_char;
  /** size of image[]  */
  public int image_size;
  /** camera image */
  public char image[];

  private static final String CARMEN_CAMERA_IMAGE_NAME = 
    "carmen_camera_image";    
  private static final String CARMEN_CAMERA_IMAGE_FMT = 
    "{int,int,int,int,<char:4>,double,string}";

  private static boolean defined = false;

  private static class PrivateCameraHandler implements IPC.HANDLER_TYPE {
    private static CameraHandler userHandler = null;
    PrivateCameraHandler(CameraHandler userHandler) {
      this.userHandler = userHandler;
    }
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      CameraMessage message = (CameraMessage)callData;
      userHandler.handleCamera(message);
    }
  }

  /** Application module calls this to subscribe to CameraMessage.
   *  Application module must extend CameraHandler
   */
  public static void subscribe(CameraHandler handler) {
    IPC.defineMsg(CARMEN_CAMERA_IMAGE_NAME, CARMEN_CAMERA_IMAGE_FMT);
    IPC.subscribeData(CARMEN_CAMERA_IMAGE_NAME, 
		      new PrivateCameraHandler(handler),
		      CameraMessage.class);
    IPC.setMsgQueueLength(CARMEN_CAMERA_IMAGE_NAME, 1);
  }

  public void publish() {
    if (!defined) {
      IPC.defineMsg(CARMEN_CAMERA_IMAGE_NAME, CARMEN_CAMERA_IMAGE_FMT);
      defined = true;
    }
    IPC.publishData(CARMEN_CAMERA_IMAGE_NAME, this);
  }
}

