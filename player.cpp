#include "player.h"

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
// TODO: Better
  return glyph('@', c_white, c_black);
}

bool Player::can_move_to(Map *map, int x, int y)
{
// TODO: Remove me, obvs
  return true;
}
