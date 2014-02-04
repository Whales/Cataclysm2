#include "attack.h"
#include "rng.h"
#include "stringfunc.h"
#include "window.h"
#include "entity.h" // For Stats
#include "item.h"
#include <sstream>

bool load_verbs(std::istream &data, std::string &verb_second,
                std::string &verb_third, std::string owner_name);

Damage_set::Damage_set()
{
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] = 0;
  }
}

Damage_set::~Damage_set()
{
}

void Damage_set::set_damage(Damage_type type, int amount)
{
  damage[type] = amount;
}

void Damage_set::set_damage(int index, int amount)
{
  if (index < 0 || index >= DAMAGE_MAX) {
    return;
  }
  damage[index] = amount;
}

int Damage_set::get_damage(Damage_type type) const
{
  return damage[type];
}

int Damage_set::get_damage(int index) const
{
  if (index < 0 || index >= DAMAGE_MAX) {
    return 0;
  }
  return damage[index];
}

int Damage_set::total_damage()
{
  int ret = 0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    ret += get_damage(i);
  }
  return ret;
}

Damage_set& Damage_set::operator+=(const Damage_set& rhs)
{
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] += rhs.get_damage(i);
  }
  return *this;
}

Damage_set& Damage_set::operator-=(const Damage_set& rhs)
{
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] -= rhs.get_damage(i);
    if (damage[i] < 0) {
      damage[i] = 0;
    }
  }
  return *this;
}

Attack::Attack()
{
  verb_second = "hit";
  verb_third = "hits";
  weight =  10;
  speed  = 100;
  to_hit =   0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] = 0;
  }
}

Attack::~Attack()
{
}

bool Attack::load_data(std::istream &data, std::string owner_name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {

    if ( ! (data >> ident) ) {
      return false;
    }

    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') { // I'ts a comment
      std::getline(data, junk);

    } else if (ident == "verb_second:") {
      std::getline(data, verb_second);
      verb_second = trim(verb_second);

    } else if (ident == "verb_third:") {
      std::getline(data, verb_third);
      verb_third = trim(verb_third);

    } else if (ident == "verb:") {
      if (!load_verbs(data, verb_second, verb_third, owner_name)) {
        return false;
      }

    } else if (ident == "weight:") {
      data >> weight;
      std::getline(data, junk);

    } else if (ident == "speed:") {
      data >> speed;
      std::getline(data, junk);

    } else if (ident == "to_hit:") {
      data >> to_hit;
      std::getline(data, junk);

    } else if (ident != "done") {
// Check if it's a damage type; TODO: make this less ugly.
// Damage_set::load_data() maybe?
      std::string damage_name = ident;
      size_t colon = damage_name.find(':');
      if (colon != std::string::npos) {
        damage_name = damage_name.substr(0, colon);
      }
      Damage_type type = lookup_damage_type(damage_name);
      if (type == DAMAGE_NULL) {
        debugmsg("Unknown Attack property '%s' (%s)",
                 ident.c_str(), owner_name.c_str());
        return false;
      } else {
        data >> damage[type];
      }
    }

  }
  return true;
}

void Attack::use_weapon(Item weapon, Stats stats)
{
  speed  += weapon.get_base_attack_speed(stats);
  to_hit += weapon.get_to_hit();
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] += weapon.get_damage( Damage_type(i) );
  }
}

Damage_set Attack::roll_damage()
{
  Damage_set ret;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    ret.set_damage( Damage_type(i), rng(0, damage[i]) );
  }
  return ret;
}

Ranged_attack::Ranged_attack()
{
  verb_second = "shoot";
  verb_third = "shoots";
  weight = 0;
  speed = 100;
  charge_time = 0;
  range = 0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] = 0;
    armor_divisor[i] = 1;
  }
}

Ranged_attack::~Ranged_attack()
{
}

bool Ranged_attack::load_data(std::istream &data, std::string owner_name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {

    if ( ! (data >> ident) ) {
      return false;
    }

    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') { // I'ts a comment
      std::getline(data, junk);

    } else if (ident == "verb_second:") {
      std::getline(data, verb_second);
      verb_second = trim(verb_second);

    } else if (ident == "verb_third:") {
      std::getline(data, verb_third);
      verb_third = trim(verb_third);

    } else if (ident == "verb:" ) {
      if (!load_verbs(data, verb_second, verb_third, owner_name)) {
        return false;
      }

    } else if (ident == "weight:") {
      data >> weight;
      std::getline(data, junk);

    } else if (ident == "speed:") {
      data >> speed;
      std::getline(data, junk);

    } else if (ident == "charge:" || ident == "charge_time:") {
      data >> charge_time;
      std::getline(data, junk);

    } else if (ident == "range:") {
      data >> range;
      std::getline(data, junk);

    } else if (ident == "variance:") {
      if (!variance.load_data(data, owner_name)) {
        return false;
      }

    } else if (ident == "armor_pierce:") {
      std::string damage_name;
      data >> damage_name;
      Damage_type damtype = lookup_damage_type(damage_name);
      if (damtype == DAMAGE_NULL) {
        debugmsg("Unknown damage type for Ranged_attack pierce: '%s' (%s)",
                 damage_name.c_str(), owner_name.c_str());
        return false;
      }
      data >> armor_divisor[damtype];
      if (armor_divisor[damtype] == 0) {
        armor_divisor[damtype] = 1;
      }

    } else if (ident != "done") {
      std::string damage_name = ident;
      size_t colon = ident.find(':');
      if (colon != std::string::npos) {
        damage_name = ident.substr(0, colon);
      }
      Damage_type type = lookup_damage_type(damage_name);
      if (type == DAMAGE_NULL) {
        debugmsg("Unknown Attack property '%s' (%s)",
                 ident.c_str(), owner_name.c_str());
        return false;
      } else {
        data >> damage[type];
      }
    }

  }
  return true;
}

int Ranged_attack::roll_variance()
{
  return variance.roll();
}

Damage_set Ranged_attack::roll_damage()
{
  Damage_set ret;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    ret.set_damage( Damage_type(i), rng(0, damage[i]) );
  }
  return ret;
}


Body_part random_body_part_to_hit()
{
  int pick = rng(1, 13);
  switch (pick) {
    case  1:  return BODYPART_HEAD;
    case  2:
    case  3:  return BODYPART_LEFT_ARM;
    case  4:
    case  5:  return BODYPART_RIGHT_ARM;
    case  6:
    case  7:  return BODYPART_LEFT_LEG;
    case  8:
    case  9:  return BODYPART_RIGHT_LEG;
    case 10:
    case 11:
    case 12:
    case 13:  return BODYPART_TORSO;
  }

  return BODYPART_TORSO;
}

bool load_verbs(std::istream &data, std::string &verb_second,
                std::string &verb_third, std::string owner_name)
{
  std::string verb_line;
  std::getline(data, verb_line);
  std::istringstream verb_data(verb_line);

  std::string tmpword;
  std::string tmpverb;
  bool reading_second = true;

  while (verb_data >> tmpword) {
    if (tmpword == "/") {
      if (!reading_second) {
        debugmsg("Too many / in verb definition (%s)", owner_name.c_str());
        return false;
      }
      reading_second = false;
      verb_second = tmpverb;
      tmpverb = "";
    } else {
      if (!tmpverb.empty()) {
        tmpverb += " ";
      }
      tmpverb += tmpword;
    }
  }

  if (reading_second) {
    debugmsg("Verb definition has 2nd person, but no 3rd", owner_name.c_str());
    return false;
  }
  verb_third = tmpverb;

  return true;
}
