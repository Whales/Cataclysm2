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
    case TOOL_ACT_DIG:    return "dig";
    case TOOL_ACT_MAX:    return "BUG - TOOL_ACT_MAX";
    default:              return "BUG - Unnamed Tool_action";
  }
  return "BUG - Escaped tool_action_name switch";
}

bool tool_action_targets_map(Tool_action action)
{
  switch (action) {
    case TOOL_ACT_PRY:
    case TOOL_ACT_DIG:
      return true;
  }
  return false;
}

Tool_target lookup_tool_target(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < TOOL_ACT_MAX; i++) {
    Tool_target ret = Tool_target(i);
    if (name == no_caps( tool_target_name(ret) )) {
      return ret;
    }
  }
  return TOOL_TARGET_NULL;
}
    
std::string tool_target_name(Tool_target action)
{
  switch (action) {
    case TOOL_TARGET_NULL:      return "NULL";
    case TOOL_TARGET_ADJACENT:  return "adjacent";
    case TOOL_TARGET_RANGED:    return "ranged";
    case TOOL_TARGET_MAX:       return "BUG - TOOL_TARGET_MAX";
    default:                    return "BUG - Unnamed Tool_target";
  }
  return "BUG - Escaped tool_target_name switch";
}
