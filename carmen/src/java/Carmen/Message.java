package Carmen;
import IPC.*;

import java.util.HashSet;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

/** Carmen generic or base Message class */

public class Message {
  public double timestamp;
  public String host;  

  private static HashSet<String> defined_messages = new HashSet<String>(); 

  /** New message has timestamp from this host and host name */
  Message() {
    timestamp = Util.getTime();
    host = Util.getHostName();
  }

  private static void verifyFormatString(String msgFormat, Class msgClass)
  {

  }

  private static void verifyFormatString(String msgFormat, Object msgInstance)
  {
    Class className = msgInstance.getClass();
    verifyFormatString(msgFormat, className);
  }

  private static class PrivateHandler implements IPC.HANDLER_TYPE {
    private Object userHandler;
    private Class handlerClass;
    private Class messageClass;

    PrivateHandler(Object userHandler, Class handlerClass, Class messageClass) 
    {
      this.userHandler = userHandler;
      this.handlerClass = handlerClass;
      this.messageClass = messageClass;
   }

    public void handle(IPC.MSG_INSTANCE msgInstance, Object callData) 
    {
      Method handleMethod;

      try {
	handleMethod = handlerClass.getMethod("handle", messageClass);
      }
      catch (NoSuchMethodException e) {
	System.err.println("You subscribed to "+messageClass+" but you used a"+
			   " handler "+handlerClass+" that doesn't\n"+
			   "have a method handle("+messageClass+")\n");
	throw new Error(e.toString());
      }
      
      try {
	handleMethod.invoke(callData);
      }
      catch (IllegalAccessException e) {
	System.err.println(e.toString());
	System.exit(-1);
      }
      catch (IllegalArgumentException e) {
	System.err.println(e.toString());
	System.exit(-1);
      }
      catch (InvocationTargetException e) {
	System.err.println(e.toString());
	System.exit(-1);
      }
    }

  }  

  protected static void subscribe(String messageName, String messageFmt, 
				  Object handler, Class messageClass) 
  {
    if (!defined_messages.contains(messageName)) {
      verifyFormatString(messageFmt, messageClass);
      IPC.defineMsg(messageName, messageFmt);
      defined_messages.add(messageName);
    }

    Class handlerClass = handler.getClass();

    try {
      Method handleMethod = handlerClass.getMethod("handle", messageClass);
    }
    catch (NoSuchMethodException e) {
      System.err.println("You subscribed to "+messageClass+" but you used a "+
			 "handler that doesn't\nhave a method \n"+
			 "handle("+messageClass+")\n");
      throw new Error(e.toString());
    }

    PrivateHandler pvtHandler = new PrivateHandler(handler, handlerClass,
						   messageClass);

    IPC.subscribeData(messageName, pvtHandler, messageClass);
  }

  public void publish(String msgName, String msgFmt, Object message) 
  {
    if (!defined_messages.contains(msgName)) {
      verifyFormatString(msgFmt, message);
      IPC.defineMsg(msgName, msgFmt);
      defined_messages.add(msgName);
    }

    IPC.publishData(msgName, this);
  }
}

