#include "entity.h"

Entity::Entity()
{
  posx = 15;
  posy = 15;
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
  return true;
}

void Entity::move_to(int x, int y)
{
  posx = x;
  posy = y;
}

Player::Player()
{
  posx = 15;
  posy = 15;
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
  return true;
  if (!map) {
    return false;
  }
  if (map->move_cost(x, y) == 0) {
    return false;
  }
  return true;
}
