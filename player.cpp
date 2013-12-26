#include "player.h"
#include <sstream>

Player::Player()
{
  posx = 15;
  posy = 15;
  action_points = 100;
  name = "Whales";
  for (int i = 0; i < BODYPART_MAX; i++) {
    current_hp[i] = 100;
    max_hp[i] = 100;
  }
}

Player::~Player()
{
}

std::string Player::get_name()
{
  return name;
}

std::string Player::get_name_to_player()
{
  return "you";
}

std::string Player::get_possessive()
{
  return "your";
}

glyph Player::get_glyph()
{
// TODO: Better
  return glyph('@', c_white, c_black);
}

int Player::get_speed()
{
// TODO: Make this use actual stuff
  return 100;
}

bool Player::can_move_to(Map *map, int x, int y)
{
// TODO: Remove me, obvs
  return true;
}

void Player::take_damage(Damage_type type, int damage, std::string reason,
                         Body_part part)
{
// TODO: Armor absorbs damage
  current_hp[part] -= damage;
}

std::string Player::hp_text(Body_part part)
{
// Sanity check
  if (part == BODYPART_MAX) {
    return "BP_MAX???";
  }
  std::stringstream ret;
  if (current_hp[part] == max_hp[part]) {
    ret << "<c=green>";
  } else {
    int percent = (100 * current_hp[part]) / max_hp[part];
    if (percent >= 75) {
      ret << "<c=ltgreen>";
    } else if (percent >= 50) {
      ret << "<c=yellow>";
    } else if (percent >= 25) {
      ret << "<c=ltred>";
    } else {
      ret << "<c=red>";
    }
  }
  ret << current_hp[part] << "<c=/>";
  return ret.str();
}
