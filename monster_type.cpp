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
  size = MON_SIZE_NULL;
  total_attack_weight = 0;
  speed = 0;
  accuracy = 0;
/* Since 0 is a valid value for dodge, we default it to -1; -1 means "hasn't
 * been set," and may be copied from a default.
 */
  dodge = -1;
  chance = 100;
  hp_set = false;
  attacks_copied_from_genus = false;
  ranged_attacks_copied_from_genus = false;
  senses_copied_from_genus = false;
  idle_sound_chance = 0;
  attack_sound_chance = 0;
  idle_sound_volume = 0;
  attack_sound_volume = 0;
  
  for (int i = 0; i < SENSE_MAX; i++) {
    senses.push_back(false);
  }

  for (int i = 0; i < DAMAGE_MAX; i++) {
    armor.push_back(0);
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
  if (size == MON_SIZE_NULL) {
    size = mg->default_values.size;
  }

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

  if (idle_sounds.empty()) {
    idle_sounds = mg->default_values.idle_sounds;
    idle_sound_chance = mg->default_values.idle_sound_chance;
    idle_sound_volume = mg->default_values.idle_sound_volume;
  }

  if (attack_sounds.empty()) {
    attack_sounds = mg->default_values.attack_sounds;
    attack_sound_chance = mg->default_values.attack_sound_chance;
    attack_sound_volume = mg->default_values.attack_sound_volume;
  }

  if (!hp_set) {  // hp_set is true if monsters.dat includes HP for this monster
    hp_dice = mg->default_values.hp_dice;
  }

  if (speed == 0) {
    speed = mg->default_values.speed;
  }

  if (accuracy == 0) {
    accuracy = mg->default_values.accuracy;
  }

  if (dodge == -1) {
    dodge = mg->default_values.dodge;
  }

  AI = mg->default_values.AI;

// Only copy senses if we haven't set any already
  bool any_senses_set = false;
  for (int i = 0; !any_senses_set && i < senses.size(); i++) {
    any_senses_set = senses[i];
  }
  if (!any_senses_set) {
    senses_copied_from_genus = true;
    senses = mg->default_values.senses;
  }

// Only copy armor if we haven't set any already
  bool any_armor_set = false;
  for (int i = 0; !any_armor_set && i < armor.size(); i++) {
    if (armor[i] > 0) {
      any_armor_set = true;
    }
  }
  if (!any_armor_set) {
    armor_copied_from_genus = true;
    armor = mg->default_values.armor;
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

    } else if (ident == "size:") {
      std::string size_text;
      std::getline(data, size_text);
      size_text = no_caps( trim( size_text ) );
      size = lookup_monster_size(size_text);
      if (size == MON_SIZE_NULL) {
        debugmsg("Unknown monster size '%s' (%s)", size_text.c_str(),
                 name.c_str());
        return false;
      }

    } else if (ident == "hp:") {
      if (!hp_dice.load_data(data, name)) {
        return false;
      }
      hp_set = true;

    } else if (ident == "armor:") {
// Reset all armor to 0, if they were copied from our genus
// (Because we want to completely override the defaults)
      if (armor_copied_from_genus) {
        armor_copied_from_genus = false;
        for (int i = 0; i < armor.size(); i++) {
          armor[i] = 0;
        }
      }
      std::getline(data, junk);
      std::string armor_ident;
      do {
        data >> armor_ident;
        armor_ident = trim( no_caps( armor_ident ) );
        if (armor_ident != "done") { 
          Damage_type dam = lookup_damage_type(armor_ident);
          if (dam == DAMAGE_NULL) {
            debugmsg("Unknown armor type '%s' (%s)", armor_ident.c_str(),
                     name.c_str());
            return false;
          }
          data >> armor[dam];
        }
      } while (armor_ident != "done");
      std::getline(data, junk);
// End of "armor:" block

    } else if (ident == "speed:") {
      data >> speed;
      std::getline(data, junk);

    } else if (ident == "accuracy:") {
      data >> accuracy;
      std::getline(data, junk);

    } else if (ident == "dodge:") {
      data >> dodge;
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
      tmpattack.type = RANGED_ATT_OTHER;  // It's a natural attack
      if (!tmpattack.load_data(data, name)) {
        debugmsg("Ranged attack failed to load (%s)", name.c_str());
        return false;
      }
      ranged_attacks.push_back(tmpattack);
      total_ranged_attack_weight += tmpattack.weight;

    } else if (ident == "ai:") {
      std::getline(data, junk);
      AI.load_data(data, name);

    } else if (ident == "idle_sound_chance:") {
      data >> idle_sound_chance;
      std::getline(data, junk);

    } else if (ident == "attack_sound_chance:") {
      data >> attack_sound_chance;
      std::getline(data, junk);

    } else if (ident == "idle_sound_volume:") {
      data >> idle_sound_volume;
      std::getline(data, junk);

    } else if (ident == "attack_sound_volume:") {
      data >> attack_sound_volume;
      std::getline(data, junk);

    } else if (ident == "idle_sounds:") {
      std::string sound_line;
      std::getline(data, sound_line);
      std::istringstream sound_data(sound_line);
      if (!idle_sounds.load_data(sound_data, name)) {
        debugmsg("'%s' failed to load idle sounds.", name.c_str());
        return false;
      }

    } else if (ident == "attack_sounds:") {
      std::string sound_line;
      std::getline(data, sound_line);
      std::istringstream sound_data(sound_line);
      if (!attack_sounds.load_data(sound_data, name)) {
        debugmsg("'%s' failed to load attack sounds.", name.c_str());
        return false;
      }

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
/* If dodge is still -1, it wasn't set by the genus OR this specific monster;
 * so set it to 0.
 */
  if (dodge < 0) {
    dodge = 0;
  }
// Likewise, if we never set size, then set it to medium (a good default)
  if (size == MON_SIZE_NULL) {
    size = MON_SIZE_MEDIUM;
  }
  return true;
}

bool Monster_type::has_sense(Sense_type sense)
{
  return senses[sense];
}

int Monster_type::get_weight()
{
  switch (size) {
    case MON_SIZE_NULL:   return     0;
    case MON_SIZE_TINY:   return    40;
    case MON_SIZE_SMALL:  return   400;
    case MON_SIZE_MEDIUM: return  1500;
    case MON_SIZE_LARGE:  return  5000;
    case MON_SIZE_HUGE:   return 20000;
  }
  return 0;
}

int Monster_type::get_volume()
{
  switch (size) {
    case MON_SIZE_NULL:   return     0;
    case MON_SIZE_TINY:   return    50;
    case MON_SIZE_SMALL:  return  1200;
    case MON_SIZE_MEDIUM: return  3000;
    case MON_SIZE_LARGE:  return 12000;
    case MON_SIZE_HUGE:   return 50000;
  }
  return 0;
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

Monster_size lookup_monster_size(std::string name)
{
  name = no_caps( trim( name ) );
  for (int i = 0; i < MON_SIZE_MAX; i++) {
    Monster_size ret = Monster_size(i);
    if (no_caps( monster_size_name(ret) ) == name) {
      return ret;
    }
  }
  return MON_SIZE_NULL;
}

std::string monster_size_name(Monster_size size)
{
  switch (size) {
    case MON_SIZE_NULL:   return "NULL";
    case MON_SIZE_TINY:   return "tiny";
    case MON_SIZE_SMALL:  return "small";
    case MON_SIZE_MEDIUM: return "medium";
    case MON_SIZE_LARGE:  return "large";
    case MON_SIZE_HUGE:   return "huge";
    case MON_SIZE_MAX:    return "BUG - MON_SIZE_MAX";
    default:              return "BUG - Unnamed Monster_size";
  }
  return "BUG - Escaped monster_size_name switch";
}
