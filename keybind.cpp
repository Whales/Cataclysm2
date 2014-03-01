#include "keybind.h"
#include "stringfunc.h"
#include "window.h"
#include "globals.h"
#include <fstream>
#include <vector>

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
  std::string keystr;
  std::vector<long> keys;
  while (!fin.eof() && fin >> keystr) {
    if (lookup_key(keystr) != 0) {
      keys.push_back( lookup_key(keystr) );
    } else if (keystr == "=") {
      std::string action_name;
      std::getline(fin, action_name);
      action_name = trim(action_name);
      Interface_action act = lookup_interface_action(action_name);
      if (act == IACTION_NULL) {
        debugmsg("Unknown action in '%s': '%s'", filename.c_str(),
                 action_name.c_str());
      } else {
        for (int i = 0; i < keys.size(); i++) {
          Interface_action already_bound = bound_to_key( keys[i] );
          if (already_bound != IACTION_NULL) {
            debugmsg("Key %c bound to %s; reassigned to %s. (%s)", keys[i],
                     interface_action_name(already_bound).c_str(),
                     interface_action_name(act).c_str(),
                     filename.c_str());
          }
          bind_key( keys[i], act );
        }
      }
      keys.clear();
    } else {
      for (int i = 0; i < keystr.size(); i++) {
        keys.push_back( keystr[i] );
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
    case IACTION_SELECT:                  return "select";
    case IACTION_MOVE_N:                  return "move_north";
    case IACTION_MOVE_NE:                 return "move_northeast";
    case IACTION_MOVE_E:                  return "move_east";
    case IACTION_MOVE_SE:                 return "move_southeast";
    case IACTION_MOVE_S:                  return "move_south";
    case IACTION_MOVE_SW:                 return "move_southwest";
    case IACTION_MOVE_W:                  return "move_west";
    case IACTION_MOVE_NW:                 return "move_northwest";
    case IACTION_PAUSE:                   return "pause";
    case IACTION_MOVE_UP:                 return "move_up";
    case IACTION_MOVE_DOWN:               return "move_down";
    case IACTION_PICK_UP:                 return "pick_up";
    case IACTION_OPEN:                    return "open";
    case IACTION_CLOSE:                   return "close";
    case IACTION_SMASH:                   return "smash";
    case IACTION_EXAMINE:                 return "examine";
    case IACTION_INVENTORY:               return "inventory";
    case IACTION_DROP:                    return "drop";
    case IACTION_WIELD:                   return "wield";
    case IACTION_WEAR:                    return "wear";
    case IACTION_APPLY:                   return "apply";
    case IACTION_RELOAD_EQUIPPED:         return "reload_equipped";
    case IACTION_RELOAD:                  return "reload";
    case IACTION_THROW:                   return "throw";
    case IACTION_FIRE:                    return "fire";
    case IACTION_EAT:                     return "eat";
    case IACTION_MESSAGES_SCROLL_BACK:    return "messages_scroll_back";
    case IACTION_MESSAGES_SCROLL_FORWARD: return "messages_scroll_forward";
    case IACTION_VIEW_WORLDMAP:           return "view_worldmap";
    case IACTION_QUIT:                    return "quit";
    case IACTION_SAVE:                    return "save";
    case IACTION_DEBUG:                   return "debug";
    case IACTION_MAX:                     return "BUG - IACTION_MAX";
    default:                              return "BUG - Unnamed action";
  }
  return "BUG - Escaped switch";
}

long lookup_key(std::string name)
{
  if (name == "UP") {
    return KEY_UP;
  } else if (name == "DOWN") {
    return KEY_DOWN;
  } else if (name == "LEFT") {
    return KEY_LEFT;
  } else if (name == "RIGHT") {
    return KEY_RIGHT;
  } else if (name == "ESC") {
    return KEY_ESC;
  } else if (name == "ENTER") {
    return '\n';
  } else if (name == "HOME") {
    return KEY_HOME;
  } else if (name == "END") {
    return KEY_END;
  } else if (name == "BACKSPACE" || name == "BKSP") {
    return KEY_BACKSPACE;
  } else {
    return 0;
  }
}

Point input_direction(long ch)
{
  switch (KEYBINDINGS.bound_to_key(ch)) {
    case IACTION_MOVE_N:  return Point( 0, -1);
    case IACTION_MOVE_NE: return Point( 1, -1);
    case IACTION_MOVE_E:  return Point( 1,  0);
    case IACTION_MOVE_SE: return Point( 1,  1);
    case IACTION_MOVE_S:  return Point( 0,  1);
    case IACTION_MOVE_SW: return Point(-1,  1);
    case IACTION_MOVE_W:  return Point(-1,  0);
    case IACTION_MOVE_NW: return Point(-1, -1);
    case IACTION_PAUSE:   return Point( 0,  0);
    default:              return Point(-2, -2); // Be sure to check for this!
  }
  return Point(-2, -2);
}
