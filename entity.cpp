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
