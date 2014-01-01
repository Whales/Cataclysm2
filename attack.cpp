#include "attack.h"
#include "rng.h"

Attack::Attack()
{
  verb_first = "hit";
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

void Attack::use_weapon(Item weapon, int strength, int dexterity)
{
  speed += weapon.get_base_attack_speed(strength, dexterity);
  to_hit += weapon.get_to_hit();
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] += weapon.get_damage( Damage_type(i) );
  }
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

