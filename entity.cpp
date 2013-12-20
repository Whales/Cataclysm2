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
