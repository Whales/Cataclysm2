#include "tool.h"
#include "stringfunc.h" // For trim() and no_caps()

Tool_action lookup_tool_action(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < TOOL_ACT_MAX; i++) {
    Tool_action ret = Tool_action(i);
    if (name == no_caps( tool_action_name(ret) )) {
      return ret;
    }
  }
  return TOOL_ACT_NULL;
}
    
std::string tool_action_name(Tool_action action)
{
  switch (action) {
    case TOOL_ACT_NULL:   return "NULL";
    case TOOL_ACT_PRY:    return "pry";
    case TOOL_ACT_MAX:    return "BUG - TOOL_ACT_MAX";
    default:              return "BUG - Unnamed Tool_action";
  }
  return "BUG - Escaped tool_action_name switch";
}
