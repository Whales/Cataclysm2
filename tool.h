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

bool tool_action_targets_map(Tool_action action);

// Tool_target is a list of ways that tools can select their target
enum Tool_target
{
  TOOL_TARGET_NULL = 0, // Default - doesn't use a target
  TOOL_TARGET_ADJACENT, // "adjacent" - press a direction key, apply there
  TOOL_TARGET_RANGED,   // "ranged" - target any square
  TOOL_TARGET_MAX
};

Tool_target lookup_tool_target(std::string name);
std::string tool_target_name(Tool_target target);

#endif
