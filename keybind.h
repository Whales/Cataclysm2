#ifndef _KEYBIND_H_
#define _KEYBIND_H_

#include "geometry.h"
#include <map>
#include <string>

enum Interface_action
{
  IACTION_NULL = 0,
  IACTION_SELECT,
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
  IACTION_MOVE_UP,
  IACTION_MOVE_DOWN,
// Map interaction
  IACTION_PICK_UP,
  IACTION_OPEN,
  IACTION_CLOSE,
  IACTION_SMASH,
// Inventory interaction
  IACTION_INVENTORY,
  IACTION_DROP,
  IACTION_WIELD,
  IACTION_WEAR,
  IACTION_RELOAD_EQUIPPED,
  IACTION_RELOAD,
  IACTION_THROW,
  IACTION_FIRE,
// Main interface
  IACTION_MESSAGES_SCROLL_BACK,
  IACTION_MESSAGES_SCROLL_FORWARD,
// Other interfaces
  IACTION_VIEW_WORLDMAP,
// Other
  IACTION_QUIT,
  IACTION_SAVE,
  IACTION_MAX
};

Interface_action lookup_interface_action(std::string name);
std::string interface_action_name(Interface_action action);

long lookup_key(std::string name);

struct Keybinding_pool
{
public:
  bool bind_key(long key, Interface_action action);
  Interface_action bound_to_key(long key);

  bool load_from(std::string filename);
private:
  std::map<long,Interface_action> bindings;
};
  
Point input_direction(long ch); // Returns "one step in direction bound to ch"

#endif
