#include "entity.h"
#include "rng.h"

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

void Entity::attack(Entity* target)
{
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
