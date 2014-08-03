#include "field.h"
#include "attack.h"
#include "rng.h"
#include "stringfunc.h"
#include "window.h"
#include "entity.h" // For Stats
#include "item.h"
#include "globals.h" // For FIELDS
#include "game.h"
#include <sstream>

bool load_verbs(std::istream &data, std::string &verb_second,
                std::string &verb_third, std::string owner_name);

Attack::Attack()
{
  verb_second = "hit";
  verb_third = "hits";
  weight =  10;
  speed  = 100;
  to_hit =   0;
  using_weapon = false;
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
  if (!weapon.is_real()) {
    return;
  }
  using_weapon = true;
  speed  += weapon.get_base_attack_speed(stats);
  to_hit += weapon.get_to_hit();
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] += weapon.get_damage( Damage_type(i) );
  }
}

void Attack::adjust_with_stats(Stats stats)
{
// Strength is improved with bashing skill
// Dexterity, and a bit of perception, improve piercing skill
// All three improve cutting skill
  int stat_bash = stats.strength;
  int stat_cut  = (stats.strength + stats.dexterity * 2 + stats.perception) / 4;
  int stat_pierce = (stats.dexterity * 3 + stats.perception) / 4;

  int bash_adjust   = ( damage[DAMAGE_BASH]   * stat_bash   ) / 10;
  int cut_adjust    = ( damage[DAMAGE_CUT]    * stat_cut    ) / 10;
  int pierce_adjust = ( damage[DAMAGE_PIERCE] * stat_pierce ) / 10;

  damage[DAMAGE_BASH]   = rng(damage[DAMAGE_BASH],   bash_adjust  );
  damage[DAMAGE_CUT]    = rng(damage[DAMAGE_CUT],    cut_adjust   );
  damage[DAMAGE_PIERCE] = rng(damage[DAMAGE_PIERCE], pierce_adjust);

// We randomly reduce dexterity twice here... and then of course to_hit is
// randomly reduced when we roll to hit.  So dexterity helps but isn't a HUGE
// impact.
  int to_hit_bonus = rng(0, stats.dexterity);
  to_hit += rng(0, to_hit_bonus);
}

void Attack::adjust_with_skills(Skill_set skills)
{
// Weapon or not, Melee skill makes the attack faster & more accurate.
// 1 points of melee skill = 2 points speed improvement
  int melee = skills.get_level(SKILL_MELEE);
  speed -= (speed * melee) / 50;
// 4 points of melee skill = 1 point speed improvement, capped at +5
  to_hit += (melee > 20 ? 5 : melee / 4);
// Melee skill between multiples of 5 MAY provide a +1 boost
  if (melee < 20 && rng(1, 5) <= (melee % 5)) {
    to_hit++;
  }

  if (!using_weapon) { // Unarmed attack.
    int unarmed = skills.get_level(SKILL_UNARMED);
    damage[DAMAGE_BASH] += (unarmed > 3 ? 3 : unarmed);
    int bash_adjust = damage[DAMAGE_BASH];
    if (unarmed > 2) {
      bash_adjust = (damage[DAMAGE_BASH] * unarmed) / 2.5;
// If bash_adjust isn't a whole number, round it randomly
      int bonus = (damage[DAMAGE_BASH] * unarmed * 10) / 2.5;
      bonus = bonus % 10;
      if (rng(1, 10) <= bonus) {
        bash_adjust++;
      }
    }
    damage[DAMAGE_BASH] = rng(damage[DAMAGE_BASH], bash_adjust);

    to_hit += (unarmed > 10 ? 5 : unarmed / 2);
    if (unarmed % 2 == 1 && one_in(2)) {
      to_hit++; // Odd numbers impact hit bonus too!
    }

    speed -= (unarmed > 15 ? 30 : unarmed * 2);
    return;
  }

// Below here, we're using a weapon.
  int sk_bash   = skills.get_level(SKILL_BASH),
      sk_cut    = skills.get_level(SKILL_CUT),
      sk_pierce = skills.get_level(SKILL_PIERCE);

// Look at damage to determine if skills affect something other than damage
  if (damage[DAMAGE_BASH] >= 8 &&
      damage[DAMAGE_BASH] >= damage[DAMAGE_CUT] &&
      damage[DAMAGE_BASH] >= damage[DAMAGE_PIERCE]) {
    speed -= (sk_bash > 10 ? 30 : sk_bash * 3);

  } else if (damage[DAMAGE_CUT] >= 8 &&
      damage[DAMAGE_CUT] >= damage[DAMAGE_BASH] &&
      damage[DAMAGE_CUT] >= damage[DAMAGE_PIERCE]) {
    speed -= (sk_cut > 10 ? 20 : sk_pierce * 2);
    if (sk_cut > 18) {
      to_hit += 3;
    } else {
      to_hit += sk_cut / 6;
      if (rng(0, 6) < sk_cut % 6) {
        to_hit++;
      }
    }

  } else if (damage[DAMAGE_PIERCE] >= 8 &&
      damage[DAMAGE_PIERCE] >= damage[DAMAGE_BASH] &&
      damage[DAMAGE_PIERCE] >= damage[DAMAGE_CUT]) {
    speed -= (sk_pierce > 10 ? 30 : sk_pierce * 3);
    if (sk_pierce > 18) {
      to_hit += 6;
    } else {
      to_hit += sk_pierce / 3;
      if (rng(0, 3) < sk_pierce % 3) {
        to_hit++;
      }
    }
  }

// We can up to double our damage!
  int bash_adj   = (damage[DAMAGE_BASH]   * (sk_bash   + 1)) / 4;
  int cut_adj    = (damage[DAMAGE_CUT]    * (sk_cut    + 1)) / 4;
  int pierce_adj = (damage[DAMAGE_PIERCE] * (sk_pierce + 1)) / 4;

// Piercing doesn't get as strong a damage bonus, but a bigger speed bonus
  if (pierce_adj > damage[DAMAGE_PIERCE]) {
    pierce_adj = (damage[DAMAGE_PIERCE] + pierce_adj) / 2;
  }

  int final_bash, final_cut, final_pierce;
  if (bash_adj < damage[DAMAGE_BASH]) {
    if (!one_in(3)) {
      final_bash = rng(bash_adj, damage[DAMAGE_BASH]);
    }
  } else {
    final_bash   = rng(damage[DAMAGE_BASH],   bash_adj   );
  }
  if (cut_adj < damage[DAMAGE_CUT]) {
    if (!one_in(3)) {
      final_cut = rng(cut_adj, damage[DAMAGE_CUT]);
    }
  } else {
    final_cut   = rng(damage[DAMAGE_CUT],   cut_adj   );
  }
  if (pierce_adj < damage[DAMAGE_PIERCE]) {
    if (!one_in(3)) {
      final_pierce = rng(cut_adj, damage[DAMAGE_PIERCE]);
    }
  } else {
    final_pierce   = rng(damage[DAMAGE_PIERCE],   cut_adj   );
  }

// Don't do more than double our damage.  Skills higher than 7 still matter;
// they give a better chance of that doubling!

  if (final_bash > damage[DAMAGE_BASH] * 2) {
    damage[DAMAGE_BASH] *= 2;
  } else {
    damage[DAMAGE_BASH] = final_bash;
  }
  if (final_cut > damage[DAMAGE_CUT] * 2) {
    damage[DAMAGE_CUT] *= 2;
  } else {
    damage[DAMAGE_CUT] = final_cut;
  }
  if (final_pierce > damage[DAMAGE_PIERCE] * 2) {
    damage[DAMAGE_PIERCE] *= 2;
  } else {
    damage[DAMAGE_PIERCE] = final_pierce;
  }
}

Damage_set Attack::roll_damage(Melee_hit_type hit_type)
{
  Damage_set ret;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    if (hit_type == MELEE_HIT_GRAZE) {
      ret.set_damage( Damage_type(i), rng(0, damage[i] * .5) );
    } else if (hit_type == MELEE_HIT_CRITICAL) {
      Dice dam_dice(damage[i], 4);
      ret.set_damage( Damage_type(i), dam_dice.roll() );
    } else {
      ret.set_damage( Damage_type(i), rng(damage[i] * .5, damage[i]) );
    }
  }
  return ret;
}

int Attack::roll_damage_type(Damage_type type, Melee_hit_type hit_type)
{
  Damage_set ret = roll_damage(hit_type);
  return ret.get_damage(type);
}

Ranged_attack::Ranged_attack()
{
  verb_second = "shoot";
  verb_third = "shoots";
  weight = 10;
  speed = 100;
  charge_time = 0;
  range = 0;
  rounds = 1;
  pellets = 1;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] = 0;
    armor_divisor[i] = 10;
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

    } else if (ident == "rounds:") {
      data >> rounds;
      std::getline(data, junk);

    } else if (ident == "pellets:") {
      data >> pellets;
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
        armor_divisor[damtype] = 10;
      }

    } else if (ident == "wake_field:") {
      if (!wake_field.load_data(data, owner_name + " " + verb_third)) {
        debugmsg("Failed to load wake_field (%s)", owner_name.c_str());
        return false;
      }

    } else if (ident == "target_field:") {
      if (!target_field.load_data(data, owner_name + " " + verb_third)) {
        debugmsg("Failed to load target_field (%s)", owner_name.c_str());
        return false;
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

Damage_set Ranged_attack::roll_damage(Ranged_hit_type hit)
{
  double min, max;
  switch (hit) {
    case RANGED_HIT_NULL: // Shouldn't ever happen
      min = 1.0;
      max = 1.0;
      break;
    case RANGED_HIT_GRAZE:
      min = 0.0;
      max = 1.0;
      break;
    case RANGED_HIT_NORMAL: // Default
      min = 0.8;
      max = 1.0;
      break;
    case RANGED_HIT_CRITICAL:
      min = 1.0;
      max = 1.5;
      break;
    case RANGED_HIT_HEADSHOT:
      min = 3.0;
      max = 5.0;
      break;
  }
      
  Damage_set ret;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    ret.set_damage( Damage_type(i), rng(damage[i] * min, damage[i] * max) );
  }
  return ret;
}


Body_part random_body_part_to_hit()
{
  int pick = rng(1, 38);
  if (pick <= 4) {
    return BODY_PART_HEAD;
  }
  if (pick <= 16) {
    return BODY_PART_TORSO;
  }
  if (pick == 17) {
    return BODY_PART_LEFT_HAND;
  }
  if (pick == 18) {
    return BODY_PART_RIGHT_HAND;
  }
  if (pick <= 22) {
    return BODY_PART_LEFT_ARM;
  }
  if (pick <= 26) {
    return BODY_PART_RIGHT_ARM;
  }
  if (pick == 27) {
    return BODY_PART_LEFT_FOOT;
  }
  if (pick == 28) {
    return BODY_PART_RIGHT_FOOT;
  }
  if (pick <= 33) {
    return BODY_PART_LEFT_LEG;
  }
  return BODY_PART_RIGHT_LEG;
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
