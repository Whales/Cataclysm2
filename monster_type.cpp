#include "window.h"
#include "stringfunc.h"
#include "monster_type.h"
#include "globals.h"
#include "rng.h"
#include <sstream>

Monster_type::Monster_type()
{
  genus = NULL;
  name = "Unknown";
  uid = -1;
  sym = glyph();
  total_attack_weight = 0;
  speed = 0;
  chance = 100;
  hp_set = false;
  attacks_copied_from_genus = false;
  ranged_attacks_copied_from_genus = false;
  senses_copied_from_genus = false;
  
  for (int i = 0; i < SENSE_MAX; i++) {
    senses.push_back(false);
  }
}

Monster_type::~Monster_type()
{
}

void Monster_type::set_genus(Monster_genus *mg)
{
  if (!mg) {
    genus = NULL;
    return;
  }

  genus = mg;
// Copy any unset values from the genus
  if (attacks.empty()) {
    attacks = mg->default_values.attacks;
    if (!attacks.empty()) {
      attacks_copied_from_genus = true;
    }
    total_attack_weight = mg->default_values.total_attack_weight;
  }
  if (ranged_attacks.empty()) {
    ranged_attacks = mg->default_values.ranged_attacks;
    if (!ranged_attacks.empty()) {
      ranged_attacks_copied_from_genus = true;
    }
    total_ranged_attack_weight = mg->default_values.total_ranged_attack_weight;
  }
  if (!hp_set) {
    hp_dice = mg->default_values.hp_dice;
  }
  if (speed == 0) {
    speed = mg->default_values.speed;
  }
  AI = mg->default_values.AI;

  bool any_senses_set = false;
  for (int i = 0; !any_senses_set && i < senses.size(); i++) {
    any_senses_set = senses[i];
  }
  if (!any_senses_set) {
    senses_copied_from_genus = true;
    senses = mg->default_values.senses;
  }
    
}

void Monster_type::assign_uid(int id)
{
  uid = id;
}

std::string Monster_type::get_data_name()
{
  return name;
}

std::string Monster_type::get_name()
{
  if (display_name.empty()) {
    return name;
  }
  return display_name;
}

std::string Monster_type::get_name_plural()
{
  if (display_name_plural.empty()) {
    return get_name() + "s"; // Guess at plural.
  }
  return display_name_plural;
}

bool Monster_type::load_data(std::istream &data)
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

    } else if (ident == "name_plural:") {
      std::getline(data, display_name_plural);
      display_name_plural = trim(display_name_plural);

    } else if (ident == "genus:") {
      std::string genus_name;
      std::getline(data, genus_name);
      genus_name = trim(genus_name);
      Monster_genus *mg = MONSTER_GENERA.lookup_name(genus_name);
      if (!mg) {
        debugmsg("Unknown Monster_genus '%s' (%s)",
                 genus_name.c_str(), name.c_str());
      } else {
        set_genus(mg);
      }

    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);

    } else if (ident == "hp:") {
      hp_dice.load_data(data, name);
      hp_set = true;

    } else if (ident == "speed:") {
      data >> speed;
      std::getline(data, junk);

    } else if (ident == "chance:") {
      data >> chance;
      std::getline(data, junk);

    } else if (ident == "senses:") {
// Reset all senses to false, if they were copied from our genus
// (Because we want to completely override the defaults)
      if (senses_copied_from_genus) {
        senses_copied_from_genus = false;
        for (int i = 0; i < senses.size(); i++) {
          senses[i] = false;
        }
      }
      std::string sense_line;
      std::getline(data, sense_line);
      std::istringstream sense_data(sense_line);

      std::string sense_name;
      while (sense_data >> sense_name) {
        senses[ lookup_sense_type(sense_name) ] = true;
      }

    } else if (ident == "attack:") {
// Remove all attacks, if they were copied from our genus
// (Because we want to completely override the defaults)
      if (attacks_copied_from_genus) {
        attacks_copied_from_genus = false;
        attacks.clear();
      }
      std::getline(data, junk);
      Attack tmpattack;
      if (!tmpattack.load_data(data, name)) {
        debugmsg("Attack failed to load (%s)", name.c_str());
        return false;
      }
      attacks.push_back(tmpattack);
      total_attack_weight += tmpattack.weight;

    } else if (ident == "ranged_attack:") {
// Remove all ranged attacks, if they were copied from our genus
// (Because we want to completely override the defaults)
      if (ranged_attacks_copied_from_genus) {
        ranged_attacks_copied_from_genus = false;
        ranged_attacks.clear();
      }
      std::getline(data, junk);
      Ranged_attack tmpattack;
      if (!tmpattack.load_data(data, name)) {
        debugmsg("Ranged attack failed to load (%s)", name.c_str());
        return false;
      }
      ranged_attacks.push_back(tmpattack);
      total_ranged_attack_weight += tmpattack.weight;

    } else if (ident == "ai:") {
      std::getline(data, junk);
      AI.load_data(data, name);

    } else if (ident != "done") {
      debugmsg("Unknown monster property '%s' (%s)",
               ident.c_str(), name.c_str());
      return false;
    }
  }
// Don't add ourselfs to the genus until *after* we've loaded our values!
  if (genus) {
    genus->add_member(this);
  }
  return true;
}

bool Monster_type::has_sense(Sense_type sense)
{
  return senses[sense];
}

Monster_genus::Monster_genus()
{
  uid = -1;
  total_chance = 0;
}

Monster_genus::~Monster_genus()
{
}

void Monster_genus::add_member(Monster_type* member)
{
  if (!member) {
    return;
  }
  total_chance += member->chance;
  members.push_back(member);
}

Monster_type* Monster_genus::random_member()
{
  if (members.empty()) {
    return NULL;
  }
  int index = rng(1, total_chance);
  for (int i = 0; i < members.size(); i++) {
    index -= members[i]->chance;
    if (index <= 0) {
      return members[i];
    }
  }
  return members.back();
}

void Monster_genus::assign_uid(int id)
{
  uid = id;
}

std::string Monster_genus::get_data_name()
{
  return name;
}

std::string Monster_genus::get_name()
{
  if (display_name.empty()) {
    return name;
  }
  return display_name;
}

bool Monster_genus::load_data(std::istream &data)
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

    } else if (ident == "default:") {
      if (!default_values.load_data(data)) {
        debugmsg("Genus %s failed to load default values", name.c_str());
        return false;
      }

    } else if (ident != "done") {
      debugmsg("Unknown monster genus property '%s' (%s)",
               ident.c_str(), name.c_str());
      return false;
    }
  }
  return true;
}
