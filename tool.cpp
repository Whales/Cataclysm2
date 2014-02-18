#include "tool.h"
#include "stringfunc.h" // For trim() and no_caps()
#include "window.h" // For debugmsg()

Tool_special_explosion::Tool_special_explosion()
{
  radius = Dice(0, 0,  8);
  damage = Dice(0, 0, 20);
}

bool Tool_special_explosion::load_data(std::istream& data,
                                       std::string owner_name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident)) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "radius:") {
      if (!radius.load_data(data, owner_name + " explosion")) {
        return false;
      }

    } else if (ident == "damage:") {
      if (!damage.load_data(data, owner_name + " explosion")) {
        return false;
      }

    } else if (ident != "done") {
      debugmsg("Unknown Tool_special_explosion flag '%s' (%s)",
               ident.c_str(), owner_name.c_str());
      return false;
    }
  }
  return true;
}

Tool_special_light::Tool_special_light()
{
  light = 8;
}

bool Tool_special_light::load_data(std::istream& data, std::string owner_name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident)) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "light:") {
      data >> light;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown Tool_special_light flag '%s' (%s)",
               ident.c_str(), owner_name.c_str());
      return false;
    }
  }
  return true;
}
  
Tool_action::Tool_action()
{
  signal = "";
  special = NULL;
  target  = TOOL_TARGET_NULL;
  ap_cost = 100;
  charge_cost = 1;
  range = 0;
  real = false;
}

Tool_action::~Tool_action()
{
  if (special) {
    delete special;
  }
}

bool Tool_action::load_data(std::istream& data, std::string owner_name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident)) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "signal:") {
      std::getline(data, signal);
      signal = no_caps(signal);
      signal = trim(signal);
      if (signal.empty()) {
        debugmsg("Empty signal (%s)", owner_name.c_str());
        return false;
      }

    } else if (ident == "special:") {
      std::string special_name;
      std::getline(data, special_name);
      special_name = no_caps(special_name);
      special_name = trim(special_name);
      if (special_name == "explosion") {
        special = new Tool_special_explosion;
      } else if (special_name == "light") {
        special = new Tool_special_light;
      } else {
        debugmsg("Unknown Tool_special '%s' (%s)", special_name.c_str(),
                 owner_name.c_str());
        return false;
      }
      if (!special->load_data(data, owner_name + " tool_action")) {
        delete special;
        special = NULL;
        return false;
      }

    } else if (ident == "target:") {
      std::string target_name;
      std::getline(data, target_name);
      target_name = trim(target_name);
      target = lookup_tool_target(target_name);
      if (target == TOOL_TARGET_NULL) {
        debugmsg("Unknown tool target '%s' (%s)", target_name.c_str(),
                 owner_name.c_str());
        return false;
      }

    } else if (ident == "ap_cost:") {
      data >> ap_cost;
      std::getline(data, junk);

    } else if (ident == "charge_cost:") {
      data >> charge_cost;
      std::getline(data, junk);

    } else if (ident == "range:") {
      data >> range;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown Tool_action property '%s' (%s)", ident.c_str(),
               owner_name.c_str());
      return false;
    }
  }
  real = true;
  return true;
}

Tool_target lookup_tool_target(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < TOOL_TARGET_MAX; i++) {
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
    case TOOL_TARGET_ALL:       return "all";
    case TOOL_TARGET_MAX:       return "BUG - TOOL_TARGET_MAX";
    default:                    return "BUG - Unnamed Tool_target";
  }
  return "BUG - Escaped tool_target_name switch";
}
