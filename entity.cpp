#include "entity.h"

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

bool Entity::can_sense(Map* map, int x, int y)
{
// Default Entity function just uses sight
  return map->senses(posx, posy, x, y);
}

Player::Player()
{
  posx = 15;
  posy = 15;
  action_points = 100;
  name = "Whales";
}

Player::~Player()
{
}

std::string Player::get_name()
{
  return name;
}

glyph Player::get_glyph()
{
  return glyph('@', c_white, c_black);
}

bool Player::can_move_to(Map *map, int x, int y)
{
// TODO: Remove me, obvs
  return true;
}
