package Carmen;

import java.util.HashMap;
import IPC.*;
/**  Carmen class for handling robot parameters in file carmen.ini 
   */
public class Param {
  private static boolean started = false;
  private static HashMap handlerMap;

  private static final String CARMEN_PARAM_VARIABLE_CHANGE_NAME = 
    "carmen_param_variable_change";
  private static final String CARMEN_PARAM_VARIABLE_CHANGE_FMT =
    "{string, string, string, int, double, [char:10]}";

  private static final String CARMEN_PARAM_QUERY_NAME =
    "carmen_param_query_string";
  private static final String CARMEN_PARAM_QUERY_FMT = 
    "{string, string, double, [char:10]}";

  private static final String CARMEN_PARAM_RESPONSE_STRING_NAME =
    "carmen_param_respond_string";
  private static final String CARMEN_PARAM_RESPONSE_STRING_FMT =
    "{string, string, string, int, double, [char:10]}";

  private static final String CARMEN_PARAM_SET_NAME =
    "carmen_param_set";
  private static final String CARMEN_PARAM_SET_FMT =
    "{string, string, string, double, [char:10]}";

  private static class ParamPrivateParamChange implements IPC.HANDLER_TYPE {
    ParamPrivateParamChange() {
    }
    /** handles parameter changes */
    public void handle (IPC.MSG_INSTANCE msgInstance, Object callData) {
      ParamChangeMessage message = (ParamChangeMessage)callData;
      String key = new 
	StringBuffer().append(message.moduleName).append(message.variableName).toString();
      ParamChangeHandler userHandler = 
	(ParamChangeHandler)handlerMap.get(key);
      if (userHandler != null) {
	userHandler.handleParamChange(message.moduleName, 
				      message.variableName, message.newValue);
      }
    }
  }
  /** Class method that handles query of current parameter values by module and variable */
  public static class ParamQuery {
    public String moduleName;
    public String variableName;
    public double timestamp;
    public char host[];

    ParamQuery(String moduleName, String variableName) {
      this.moduleName = moduleName;
      this.variableName = variableName;
      timestamp = Util.getTimeMs();
      host = Util.getHostName();
    }
  }
  /** inner class of Param */
  public static class ParamResponse {
    public String moduleName;
    public String variableName;
    public String value;
    public int status;
    public double timestamp;
    public char host[];
  }

  /** inner class of Param */
  public static class ParamSet {
    public String moduleName;
    public String variableName;
    public String value;
    public double timestamp;
    public char host[];

    ParamSet(String moduleName, String variableName, String value) {
      this.moduleName = moduleName;
      this.variableName = variableName;
      this.value = value;
      timestamp = Util.getTimeMs();
      host = Util.getHostName();
    }
  }

  /** Class method for parameter queryring */
  public static String query(String moduleName, String variableName) {
    IPC.defineMsg(CARMEN_PARAM_QUERY_NAME, CARMEN_PARAM_QUERY_FMT);
    IPC.defineMsg(CARMEN_PARAM_RESPONSE_STRING_NAME, 
		  CARMEN_PARAM_RESPONSE_STRING_FMT);
    ParamQuery query = new ParamQuery(moduleName, variableName);
    ParamResponse response = (ParamResponse)IPC.queryResponseData
      (CARMEN_PARAM_QUERY_NAME, query, ParamResponse.class, 5000);
    return response.value;
  }
  /** Class method to set a variable with a new value */
  public static boolean set(String moduleName, String variable, 
			    String newValue) {
    ParamSet msg = new ParamSet(moduleName, variable, newValue);
    ParamResponse response = (ParamResponse)IPC.queryResponseData
      (CARMEN_PARAM_SET_NAME, msg, ParamResponse.class, 5000);
    if (response.status == 0)
      return true;
    return false;
  }

  /** Class method for subscribing to parameter data */
  public static void subscribe(String moduleName, String variable, 
			       ParamChangeHandler handler) {
    if (!started) {
      IPC.defineMsg(CARMEN_PARAM_VARIABLE_CHANGE_NAME, 
		    CARMEN_PARAM_VARIABLE_CHANGE_FMT);
      IPC.subscribeData(CARMEN_PARAM_VARIABLE_CHANGE_NAME, 
			new ParamPrivateParamChange(),
			ParamChangeMessage.class);
      handlerMap = new HashMap();
      started = true;
    }
    String key = new 
      StringBuffer().append(moduleName).append(variable).toString();
    handlerMap.put(key, handler);
  }
  
  public static boolean parseBoolean(String value) {
    if (value.equalsIgnoreCase("1"))
      return true;
    return false;
  }

  public static boolean parseOnoff(String value) {
    if (value.equalsIgnoreCase("on"))
      return true;
    return false;
  }
}
