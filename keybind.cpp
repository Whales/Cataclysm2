#include "keybind.h"
#include "stringfunc.h"
#include "window.h"
#include <fstream>

bool Keybinding_pool::bind_key(long key, Interface_action action)
{
  bindings[key] = action;
  return true;
}

Interface_action Keybinding_pool::bound_to_key(long key)
{
  if (bindings.count(key) == 0) {
    return IACTION_NULL;
  }
  return bindings[key];
}

bool Keybinding_pool::load_from(std::string filename)
{
  std::ifstream fin;
  fin.open(filename.c_str());
  if (!fin.is_open()) {
    debugmsg("Failed to open '%s'", filename.c_str());
    return false;
  }
  std::string keys;
  while (!fin.eof() && fin >> keys) {
    std::string action_name;
    std::getline(fin, action_name);
    action_name = trim(action_name);
    Interface_action act = lookup_interface_action(action_name);
    if (act == IACTION_NULL) {
      debugmsg("Bad action in '%s': '%s'", filename.c_str(),
               action_name.c_str());
    } else {
      for (int i = 0; i < keys.size(); i++) {
        bind_key( keys[i], act );
      }
    }
  }
  return true;
}

Interface_action lookup_interface_action(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < IACTION_MAX; i++) {
    Interface_action ret = Interface_action(i);
    if ( no_caps( interface_action_name(ret) ) == name ) {
      return ret;
    }
  }
  return IACTION_NULL;
}

std::string interface_action_name(Interface_action action)
{
  switch (action) {
    case IACTION_NULL:                    return "NULL";
    case IACTION_MOVE_N:                  return "move_north";
    case IACTION_MOVE_NE:                 return "move_northeast";
    case IACTION_MOVE_E:                  return "move_east";
    case IACTION_MOVE_SE:                 return "move_southeast";
    case IACTION_MOVE_S:                  return "move_south";
    case IACTION_MOVE_SW:                 return "move_southwest";
    case IACTION_MOVE_W:                  return "move_west";
    case IACTION_MOVE_NW:                 return "move_northwest";
    case IACTION_PAUSE:                   return "pause";
    case IACTION_PICK_UP:                 return "pick_up";
    case IACTION_INVENTORY:               return "inventory";
    case IACTION_DROP:                    return "drop";
    case IACTION_WIELD:                   return "wield";
    case IACTION_MESSAGES_SCROLL_BACK:    return "messages_scroll_back";
    case IACTION_MESSAGES_SCROLL_FORWARD: return "messages_scroll_forward";
    case IACTION_VIEW_WORLDMAP:           return "view_worldmap";
    case IACTION_QUIT:                    return "quit";
    case IACTION_SAVE:                    return "save";
    case IACTION_MAX:                     return "BUG - IACTION_MAX";
    default:                              return "BUG - Unnamed action";
  }
  return "BUG - Escaped switch";
}
