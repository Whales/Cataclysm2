#include "entity.h"
#include "rng.h"
#include "game.h"
#include <sstream>

Entity::Entity()
{
  posx = 15;
  posy = 15;
  action_points = 0;
}

Entity::~Entity()
{
}

std::string Entity::get_name()
{
  return "Nothing";
}

std::string Entity::get_name_to_player()
{
  return "Nothing";
}

std::string Entity::get_possessive()
{
  return "Nothing's";
}

glyph Entity::get_glyph()
{
  return glyph();
}

void Entity::gain_action_points()
{
  action_points += get_speed();
}

int Entity::get_speed()
{
  return 100;
}

bool Entity::can_move_to(Map* map, int x, int y)
{
  if (!map) {
    return false;
  }
  if (map->move_cost(x, y) == 0) {
    return false;
  }
  return true;
}

void Entity::move_to(Map* map, int x, int y)
{
  posx = x;
  posy = y;
  if (map) {
    action_points -= map->move_cost(x, y);
  }
}

void Entity::pause()
{
  action_points -= 100;
}

bool Entity::add_item(Item item)
{
  inventory.push_back(item);
  return true;
}

Attack Entity::base_attack()
{
  return Attack();
}
  
void Entity::attack(Entity* target)
{
  if (!target) {
    debugmsg("'%s' attempted attack() on a null target.");
    return;
  }

  Attack att = base_attack();
  if (weapon.is_real()) {
    att.use_weapon(weapon, 0, 0); // TODO : Use stats here
  }

  action_points -= att.speed;

  bool you_see = GAME.player->can_sense(GAME.map, posx, posy);
  bool attacker_is_you = is_you();

  std::string miss_verb = (attacker_is_you ? "miss" : "misses");

  if (hit_roll(att.to_hit) < target->dodge_roll()) {
    if (you_see) {
      std::stringstream msg;
      msg << get_name_to_player() << " " << miss_verb << " " <<
             target->get_name_to_player() << "!";
      GAME.add_msg( msg.str().c_str() );
    }
// TODO: action_point penalty for missing?
    return;
  }

  Body_part bp_hit = (target->is_player() ? random_body_part_to_hit() :
                                            BODYPART_NULL);

// TODO: Should total_damage be reduced by damage absorbed by armor?
  int total_damage = 0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    int damage = rng(0, att.damage[i]);
    total_damage += damage;
    target->take_damage(Damage_type(i), damage, get_name_to_player(), bp_hit);
  }

  if (you_see) {
    std::stringstream damage_ss;
    damage_ss << get_name_to_player() << " ";
    if (attacker_is_you) {
      damage_ss << att.verb_first;
    } else {
      damage_ss << att.verb_third;
    }
    damage_ss << " ";
    if (bp_hit == BODYPART_NULL) {
      damage_ss << target->get_name_to_player();
    } else {
      damage_ss << target->get_possessive() << " " << body_part_name(bp_hit);
    }
    if (target->is_you()) {
      damage_ss << " for " << total_damage << " damage";
    }
    damage_ss << "!";
    GAME.add_msg( damage_ss.str().c_str() );
  }
}
  
int Entity::hit_roll(int bonus)
{
  return rng(1, 10) + bonus;
}

int Entity::dodge_roll()
{
  return rng(1, 10);
}

void Entity::take_damage(Damage_type type, int damage, std::string reason,
                         Body_part part)
{
}

bool Entity::can_sense(Map* map, int x, int y)
{
// Default Entity function just uses sight
  return map->senses(posx, posy, x, y);
}
