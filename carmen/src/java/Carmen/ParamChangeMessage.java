package Carmen;
/** Carmen ParamChangeHandler's message */

public class ParamChangeMessage extends Message {
  public String moduleName;
  public String variableName;
  public String newValue;
  public int status;
}

