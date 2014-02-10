#ifndef _TOOL_H_
#define _TOOL_H_

#include <string>

enum Tool_action
{
  TOOL_ACT_NULL = 0,
  TOOL_ACT_PRY,       // "pry" - Pry on adjacent tile, as a crowbar
  TOOL_ACT_MAX
};

Tool_action lookup_tool_action(std::string name);
std::string tool_action_name(Tool_action action);

#endif
