#ifndef _KEYBIND_H_
#define _KEYBIND_H_

#include <map>
#include <string>

enum Interface_action
{
  IACTION_NULL = 0,
// Movement
  IACTION_MOVE_N,
  IACTION_MOVE_NE,
  IACTION_MOVE_E,
  IACTION_MOVE_SE,
  IACTION_MOVE_S,
  IACTION_MOVE_SW,
  IACTION_MOVE_W,
  IACTION_MOVE_NW,
  IACTION_PAUSE,
// Main interface
  IACTION_MESSAGES_SCROLL_BACK,
  IACTION_MESSAGES_SCROLL_FORWARD,
// Different screens
  IACTION_VIEW_WORLDMAP,

// Other
  IACTION_QUIT,
  IACTION_SAVE,
  IACTION_MAX
};

Interface_action lookup_interface_action(std::string name);
std::string interface_action_name(Interface_action action);

struct Keybinding_pool
{
public:
  bool bind_key(long key, Interface_action action);
  Interface_action bound_to_key(long key);

  bool load_from(std::string filename);
private:
  std::map<long,Interface_action> bindings;
};
  

#endif
