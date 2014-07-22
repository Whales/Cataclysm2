#include "monster_ability.h"

Monster_ability::Monster_ability()
{
  frequency = 0;
  AP_cost = 0;
  HP_cost = 0;
}

Monster_ability::~Monster_ability()
{
}

Monster_ability_summon::Monster_ability_summon()
{
  range = 1;
  max_summons = 0;
}

Monster_ability_signal::Monster_ability_signal()
{
  range = 1;
}

Monster_ability_terrain::Monster_ability_terrain()
{
  always_replace = false;
  range = 1;
  tiles_affected = Dice(0, 0, 1);
}

Monster_ability_teleport::Monster_ability_teleport()
{
  range = 1;
  always_use_max_range = false;
  controlled = false;
  phase = false;
}

Monster_ability_fields::Monster_ability_fields()
{
  range = 1;
  affect_all_tiles = false;
}

bool Monster_ability::load_data(std::istream& data, std::string owner)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      debugmsg("Couldn't read file for loading Monster_ability (%s)",
               owner.c_str());
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment; clear the line.
      std::getline(data, junk);

    } else if (ident == "freq:" || ident == "frequency:") {
      data >> frequency;
      std::getline(data, junk);

    } else if (ident == "ap_cost:") {
      data >> AP_cost;
      std::getline(data, junk);

    } else if (ident == "hp_cost:") {
      data >> HP_cost;
      std::getline(data, junk);

    } else if (ident == "verb:") {
      std::getline(data, verb);
      verb = trim(verb);

    } else if (ident != "done" && !handle_data(ident, data)) {
      debugmsg("Unknown Monster_ability property '%s' (%s)",
               ident.c_str(), owner.c_str());
      return false;
    }
  }
  return true;
}

bool Monster_ability::handle_data(std::string ident, std::istream& data,
                                  std::string owner)
{
  return false;
}

bool Monster_ability::effect(Entity* user)
{
  debugmsg("Monster_ability::effect() called!  This is a NULL ability!");
  return false;
}

bool Monster_ability_summon::handle_data(std::string ident, std::istream& data,
                                         std::string owner)
{
  std::string junk;
  if (ident == "monster:") {
    if (!monster.load_data(data, owner)) {
      debugmsg("Failed to load monster list for Monster_ability_summon (%s)",
               owner.c_str());
      return false;
    }

  } else if (ident == "range:") {
    data >> range;
    std::getline(data, junk);

  } else if (ident == "max_summons:") {
    data >> max_summons;
    std::getline(data, junk);

  } else {
    debugmsg("Unknown Monster_ability_summon property '%s' (%s)",
             ident.c_str(), owner.c_str());
    return false;
  }
  return true;
}
