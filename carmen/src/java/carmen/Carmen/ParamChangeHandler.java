package Carmen;
/**
 * Declaration of Carmen event handler for parameter changing
 * @author RSS-CARMEN development team 
 * @version 1.0, Mar 1, 2005
 */

public interface ParamChangeHandler {
  public void handleParamChange (String moduleName, String variableName, 
				 String newValue);
}
