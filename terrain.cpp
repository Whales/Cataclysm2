#include "terrain.h"
#include "stringfunc.h"
#include "files.h"
#include "window.h"
#include "globals.h"
#include <sstream>

Terrain_smash::Terrain_smash()
{
  armor[DAMAGE_PIERCE] = Dice(0, 0, 50); // Ignore pierce by default.
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
        armor[dmgtype].load_data(data, name);
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

Terrain_signal_handler::Terrain_signal_handler()
{
  success_rate = 100;
}

// Stat_bonus loads from a single line, so it uses a different paradigm
bool Stat_bonus::load_data(std::istream& data, std::string owner_name)
{
// Get stat name
  if (!data.good()) {
    debugmsg("Stat_bonus wasn't fed data (%s)", owner_name.c_str());
    return false;
  }
  std::string stat_name;
  data >> stat_name;
  stat = lookup_stat_id(stat_name);
  if (stat == STAT_NULL) {
    debugmsg("Unknown Stat_id name '%s' (%s)",
             stat_name.c_str(), owner_name.c_str());
    return false;
  }

// Get math symbol; valid options are * > >= < <= =
  if (!data.good()) {
    debugmsg("Stat_bonus couldn't read operator (%s)", owner_name.c_str());
    return false;
  }
  std::string operator_name;
  data >> operator_name;
  op = lookup_math_operator(operator_name);
  if (op == MATH_NULL) {
    debugmsg("Unknown Math_operator '%s' (%s)",
             operator_name.c_str(), owner_name.c_str());
    return false;
  }
// Get amount
  if (!data.good()) {
    debugmsg("Stat_bonus couldn't read amount (%s)", owner_name.c_str());
    return false;
  }
  data >> amount;

// If op is a comparison operator, we need the static amount
  if (op != MATH_MULTIPLY) {  // TODO: fix this if there's more non-comp ops
    if (!data.good()) {
    debugmsg("Stat_bonus couldn't read static amount (%s)", owner_name.c_str());
      return false;
    }
    data >> amount_static;
  }
// Success!
  return true;
}

// Terrain_flag_bonus loads from a single line, so it uses a different paradigm
bool Terrain_flag_bonus::load_data(std::istream& data, std::string owner_name)
{
// Get flag name
  if (!data.good()) {
    debugmsg("Terrain_flag_bonus wasn't fed data (%s)", owner_name.c_str());
    return false;
  }
  std::string flag_name;
  data >> flag_name;
  flag = lookup_terrain_flag(flag_name);
  
  if (flag == TF_NULL) {
    debugmsg("Unknown Terrain_flag name '%s' (%s)",
             flag_name.c_str(), owner_name.c_str());
    return false;
  }

// Get amount
  if (!data.good()) {
    debugmsg("Terrain_flag_bonus couldn't read amount (%s)",
             owner_name.c_str());
    return false;
  }
  data >> amount;

// Success!
  return true;
}

bool Terrain_signal_handler::load_data(std::istream& data,
                                       std::string owner_name)
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

    } else if (ident == "result:") {
      std::getline(data, result);
      result = trim(result);

    } else if (ident == "success_rate:") {
      data >> success_rate;
      std::getline(data, junk);

    } else if (ident == "success_message:") {
      std::getline(data, success_message);
      success_message = trim(success_message);

    } else if (ident == "failure_message:") {
      std::getline(data, failure_message);
      failure_message = trim(failure_message);

    } else if (ident == "stat_bonus:") {
      std::string bonus_text;
      std::getline(data, bonus_text);
      std::istringstream bonus_data(bonus_text);
      Stat_bonus tmp;
      if (!tmp.load_data(bonus_data, owner_name + " signal handler")) {
        return false;
      }
      stat_bonuses.push_back(tmp);

    } else if (ident == "terrain_flag_bonus:") {
      std::string bonus_text;
      std::getline(data, bonus_text);
      std::istringstream bonus_data(bonus_text);
      Terrain_flag_bonus tmp;
      if (!tmp.load_data(bonus_data, owner_name + " signal handler")) {
        return false;
      }
      terrain_flag_bonuses.push_back(tmp);

    } else if (ident != "done") {
      debugmsg("Unknown terrain property '%s' (%s)",
               ident.c_str(), owner_name.c_str());
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

    } else if (ident == "destroy:") {
      std::getline(data, destroy_result);
      destroy_result = trim(destroy_result);

    } else if (ident == "signal:") {
      std::string signal_name;
      std::getline(data, signal_name);
      signal_name = no_caps(signal_name);
      signal_name = trim(signal_name);
      if (signal_handlers.count(signal_name) > 0) {
        debugmsg("Defined signal handler for '%s' twice (%s)",
                 signal_name.c_str(), name.c_str());
        return false;
      }
      Terrain_signal_handler tmphandler;
      if (!tmphandler.load_data(data, name + "::" + signal_name)) {
        return false;
      }
      signal_handlers[signal_name] = tmphandler;

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
    case TF_PLURAL:       return "plural";
    case TF_INDOORS:      return "indoors";
    case TF_SEALED:       return "sealed";
    default:              return "ERROR"; // All caps means it'll never be used
  }
  return "ERROR";
}
