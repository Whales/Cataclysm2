#include "keybind.h"
#include "stringfunc.h"
#include "window.h" // For debugmsg() and key_name()
#include "globals.h"
#include <fstream>
#include <vector>
#include <sstream>

bool Keybinding_pool::bind_key(long key, Interface_action action)
{
  bindings[key] = action;
  if (reverse_bindings.count(action) == 0) {
    reverse_bindings[action] = std::vector<long>();
  }
  reverse_bindings[action].push_back(key);
  return true;
}

Interface_action Keybinding_pool::bound_to_key(long key)
{
  if (bindings.count(key) == 0) {
    return IACTION_NULL;
  }
  return bindings[key];
}

std::vector<long> Keybinding_pool::keys_bound_to(Interface_action action)
{
  if (reverse_bindings.count(action) == 0) {
    return std::vector<long>();
  }
  return reverse_bindings[action];
}

std::string Keybinding_pool::describe_bindings_for(Interface_action action)
{
  std::vector<long> bindings = keys_bound_to(action);
  if (bindings.empty()) {
    return "nothing assigned";
  }

  std::stringstream ret;
  for (int i = 0; i < bindings.size(); i++) {
    ret << key_name(bindings[i]);
    if (i + 2 < bindings.size()) {
      ret << ", ";
    } else if (i + 1 < bindings.size()) {
      ret << " or ";
    }
  }

  return ret.str();
}

bool Keybinding_pool::bind_debug_key(long key, Debug_action action)
{
  debug_bindings[key] = action;
  return true;
}

Debug_action Keybinding_pool::bound_to_debug_key(long key)
{
  if (debug_bindings.count(key) == 0) {
    return DEBUG_ACTION_NULL;
  }
  return debug_bindings[key];
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

        Debug_action debug_act = lookup_debug_action(action_name);

        if (debug_act == DEBUG_ACTION_NULL) {
          debugmsg("Unknown action in '%s': '%s'", filename.c_str(),
                   action_name.c_str());

        } else {
          for (int i = 0; i < keys.size(); i++) {
            Debug_action already_bound = bound_to_debug_key( keys[i] );
            if (already_bound != DEBUG_ACTION_NULL) {
              debugmsg("Key %c bound to %s; reassigned to %s. (%s)", keys[i],
                       debug_action_name(already_bound).c_str(),
                       debug_action_name(debug_act).c_str(),
                       filename.c_str());
            }
            bind_debug_key( keys[i], debug_act );
          }
        }

      } else {  // if (act == IACTION_NULL)
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
  name = trim(name);
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
    case IACTION_LOOK:                    return "look";
    case IACTION_GRAB:                    return "grab";
    case IACTION_INVENTORY:               return "inventory";
    case IACTION_DROP:                    return "drop";
    case IACTION_WIELD:                   return "wield";
    case IACTION_WEAR:                    return "wear";
    case IACTION_TAKE_OFF:                return "take_off";
    case IACTION_APPLY:                   return "apply";
    case IACTION_READ:                    return "read";
    case IACTION_RELOAD_EQUIPPED:         return "reload_equipped";
    case IACTION_RELOAD:                  return "reload";
    case IACTION_THROW:                   return "throw";
    case IACTION_FIRE:                    return "fire";
    case IACTION_ADVANCE_FIRE_MODE:       return "advance_fire_mode";
    case IACTION_EAT:                     return "eat";
    case IACTION_MESSAGES_SCROLL_BACK:    return "messages_scroll_back";
    case IACTION_MESSAGES_SCROLL_FORWARD: return "messages_scroll_forward";
    case IACTION_VIEW_WORLDMAP:           return "view_worldmap";
    case IACTION_CHAR_SKILLS:             return "skills";
    case IACTION_QUIT:                    return "quit";
    case IACTION_SAVE:                    return "save";
    case IACTION_DEBUG:                   return "debug";
    case IACTION_MAX:                     return "BUG - IACTION_MAX";
    default:                              return "BUG - Unnamed action";
  }
  return "BUG - Escaped interface_action_name() switch";
}

Debug_action lookup_debug_action(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < IACTION_MAX; i++) {
    Debug_action ret = Debug_action(i);
    if ( no_caps( debug_action_name(ret) ) == name ) {
      return ret;
    }
  }
  return DEBUG_ACTION_NULL;
}

std::string debug_action_name(Debug_action action)
{
  switch (action) {
    case DEBUG_ACTION_NULL:           return "NULL";
    case DEBUG_ACTION_CREATE_ITEM:    return "create_item";
    case DEBUG_ACTION_BUILD_MAP:      return "build_map";
    case DEBUG_ACTION_MAP_INFO:       return "map_info";
    case DEBUG_ACTION_PLACE_FIELD:    return "place_field";
    case DEBUG_ACTION_CLEAR_ITEMS:    return "clear_items";
    case DEBUG_ACTION_SPAWN_MONSTER:  return "spawn_monster";
    case DEBUG_ACTION_MONSTER_PATH:   return "monster_path";
    case DEBUG_ACTION_MEMORY_INFO:    return "memory_info";
    case DEBUG_ACTION_PLACE_BONUS:    return "place_bonus";
    case DEBUG_ACTION_GAIN_XP:        return "gain_xp";
    case DEBUG_ACTION_MAX:            return "BUG - DEBUG_ACTION_MAX";
    default:                          return "BUG - Unnamed Debug_action";
  }
  return "BUG - Escaped debug_action_name() switch";
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
