#include "terrain.h"
#include "stringfunc.h"
#include "files.h"
#include "window.h"
#include "globals.h"
#include <sstream>

Terrain_smash::Terrain_smash()
{
  for (int i = 0; i < DAMAGE_MAX; i++) {
    armor[i] = 0;
  }
  armor[DAMAGE_PIERCE] = 50; // Ignore pierce by default.
  ignore_chance = 0;
}

bool Terrain_smash::load_data(std::istream &data, std::string name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "success_sound:") {
      std::getline(data, success_sound);
      success_sound = trim(success_sound);

    } else if (ident == "failure_sound:") {
      std::getline(data, failure_sound);
      failure_sound = trim(failure_sound);

    } else if (ident == "armor:") {
      std::string damage_name;
      data >> damage_name;
      Damage_type dmgtype = lookup_damage_type(damage_name);
      if (dmgtype == DAMAGE_NULL) {
        debugmsg("Invalid armor type '%s' in smash data for '%s'",
                 damage_name.c_str(), name.c_str());
        return false;
      } else {
        data >> armor[dmgtype];
        std::getline(data, junk);
      }

    } else if (ident == "ignore_chance:") {
      data >> ignore_chance;

    } else if (ident != "done") {
      debugmsg("Unknown Terrain_smash flag '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  }
  return true;
}


Terrain::Terrain()
{
  uid = -1;
  name = "ERROR";
  sym = glyph();
  movecost  = 100;
  height    =  -1;
  hp        =   0;
  smashable = false;
  for (int i = 0; i < TF_MAX; i++) {
    flags.push_back(false);
  }
}

std::string Terrain::get_data_name()
{
  return name;
}

std::string Terrain::get_name()
{
  if (display_name.empty()) {
    return name;
  }
  return display_name;
}

bool Terrain::load_data(std::istream &data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);

    } else if (ident == "display_name:") {
      std::getline(data, display_name);
      display_name = trim(display_name);

    } else if (ident == "glyph:") {
      sym.load_data_text(data, name);
      std::getline(data, junk);

    } else if (ident == "movecost:") {
      data >> movecost;
      std::getline(data, junk);

    } else if (ident == "hp:") {
      data >> hp;
      std::getline(data, junk);

    } else if (ident == "height:") {
      data >> height;
      std::getline(data, junk);

    } else if (ident == "smashable" || ident == "smash:") {
      std::getline(data, junk);
      if (smash.load_data(data, name)) {
        smashable = true;
      } else {
        smash = Terrain_smash();
      }

    } else if (ident == "open:") {
      std::getline(data, open_result);
      open_result = trim(open_result);

    } else if (ident == "close:") {
      std::getline(data, close_result);
      close_result = trim(close_result);

    } else if (ident == "destroy:") {
      std::getline(data, destroy_result);
      destroy_result = trim(destroy_result);

    } else if (ident == "receiver:") {
      std::string tool_action, tmpstr, result;
      while (tmpstr != "=") {
        data >> tmpstr;
        if (tmpstr != "=") {
          if (!tool_action.empty()) {
// Insert a space only if this isn't the first word
            tool_action += " ";
          }
          tool_action += tmpstr;
        }
      }

      tool_action = no_caps(tool_action);
      tool_action = trim(tool_action);
      if (tool_action.empty()) {
        debugmsg("Empty tool_action (%s)", name.c_str());
        return false;
      }
          
      std::getline(data, result);
      result = trim(result);
      tool_result[tool_action] = result;

    } else if (ident == "flags:") {
      std::string flag_line;
      std::getline(data, flag_line);
      std::istringstream flagdata(flag_line);
      std::string flagname;
      while (flagdata >> flagname) {
        Terrain_flag tf = lookup_terrain_flag(flagname);
        if (tf == TF_NULL) {
          debugmsg("Unknown terrain flag '%s' (%s)",
                   flagname.c_str(), name.c_str());
          return false;
        }
        flags[tf] = true;
      }

    } else if (ident != "done") {
      debugmsg("Unknown terrain property '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  }
  if (hp > 0 && destroy_result.empty()) {
    debugmsg("\
Terrain '%s' has HP %d but no destroy_result.\n\
Either set a destroy_result or omit HP line.",
             name.c_str(), hp);
    return false;
  }

// Finally, set height (if it's not set)
  if (height == -1) {
    if (movecost == 0) {
      height = 100;
    } else {
      height =   0;
    }
  }
  return true;
}

bool Terrain::has_flag(Terrain_flag flag)
{
  if (flag < 0 || flag > flags.size()) {
    debugmsg("Terrain::has_flag error - flag %d/%d, terrain %s",
             flag, flags.size(), name.c_str());
    return false;
  }
  return flags[flag];
}

Terrain_flag lookup_terrain_flag(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < TF_MAX; i++) {
    if ( terrain_flag_name( Terrain_flag(i) ) == name) {
      return Terrain_flag(i);
    }
  }
  return TF_NULL;
}

// Note: ALL terrain flag names must be all lowercase!
std::string terrain_flag_name(Terrain_flag flag)
{
  switch (flag) {
    case TF_NULL:         return "null";
    case TF_OPAQUE:       return "opaque";
    case TF_FLOOR:        return "floor";
    case TF_STAIRS_UP:    return "stairs_up";
    case TF_STAIRS_DOWN:  return "stairs_down";
    case TF_OPEN_SPACE:   return "open_space";
    case TF_WATER:        return "water";
    case TF_FLAMMABLE:    return "flammable";
    case TF_CONTAINER:    return "container";
    default:              return "ERROR"; // All caps means it'll never be used
  }
  return "ERROR";
}
