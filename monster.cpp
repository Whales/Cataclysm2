#include "monster.h"
#include "rng.h"
#include "game.h"
#include "player.h"
#include <sstream>

Monster::Monster()
{
  dead = false;
  killed_by_player = false;
  current_hp = 0;
  type = NULL;
  action_points = 0;
}

void Monster::set_type(std::string name)
{
  dead = false;
  killed_by_player = false;
  type = MONSTER_TYPES.lookup_name(name);
  if (type) {
    current_hp = rng(type->minimum_hp, type->maximum_hp);
  }
  action_points = 0;
}

Monster::~Monster()
{
}

glyph Monster::get_glyph()
{
  if (type) {
    return type->sym;
  }
  return glyph();
}

std::string Monster::get_name()
{
  if (type) {
    return type->name;
  }
  return "Typeless Monster";
}

std::string Monster::get_name_to_player()
{
  return get_name_definite();
}

std::string Monster::get_possessive()
{
  std::stringstream ret;
  ret << get_name_definite() << "'s";
  return ret.str();
}

std::string Monster::get_name_indefinite()
{
// TODO: more complicated
  std::stringstream ret;
  ret << "a " << get_name();
  return ret.str();
}

std::string Monster::get_name_definite()
{
  std::stringstream ret;
  ret << "the " << get_name();
  return ret.str();
}

bool Monster::has_sense(Sense_type sense)
{
  if (!sense) {
    return false;
  }
  return type->has_sense(sense);
}

int Monster::get_speed()
{
  if (!type) {
    return 0;
  }
  return type->speed;
}

void Monster::make_plans()
{
  Player *player = GAME.player;
  Map *map = GAME.map;
// TODO: Support different senses
// TODO: Support non-aggressive monsters
  bool senses_player = false;
  if (has_sense(SENSE_SIGHT) && can_sense(map, player->posx, player->posy)) {
    senses_player = true;
  }
  if (senses_player) {
    target = player;
    wander_target = Point(player->posx, player->posy);
// TODO: Don't hard-code wander_duration.  Make it a Monster_type stat?
    wander_duration = 15;
  } else {
    target = NULL;
  }
}

void Monster::take_turn()
{
// TODO: Move make_plans() outside of this function?
  make_plans();
  while (action_points > 0 && !dead) {
    if (wander_duration > 0) {
      wander_duration--;
    }
    if (target) {
      if (can_attack(target)) {
        attack(target);
      } else {
        move_towards(target);
      }
    } else {
      wander();
    }
  }
}

bool Monster::can_attack(Entity* entity)
{
  if (!entity) {
    return false;
  }
  if (rl_dist(posx, posy, entity->posx, entity->posy) <= 1) {
    return true;
  }
  return false;
}

Attack Monster::base_attack()
{
  if (!type || type->attacks.empty()) {
    return Attack();
  }
  int index = rng(1, type->total_attack_weight);
  for (int i = 0; i < type->attacks.size(); i++) {
    index -= type->attacks[i].weight;
    if (index <= 0) {
      return type->attacks[i];
    }
  }
  return type->attacks.back();
}

void Monster::take_damage(Damage_type type, int damage, std::string reason,
                          Body_part part)
{
  current_hp -= damage;
  if (current_hp <= 0) {
    dead = true;
    if (reason == "you") {
      killed_by_player = true;
    }
  }
}

void Monster::move_towards(Entity* entity)
{
  if (!entity) {
    debugmsg("Monster attempted move_towards() on a null target.");
    return;
  }
  move_towards(entity->posx, entity->posy);
}

void Monster::move_towards(int target_x, int target_y)
{
  int dx = target_x - posx, dy = target_y - posy;
  int ax = abs(dx), ay = abs(dy);
  bool x_diff_bigger = false;
  if (ax > ay || (ax == ay && one_in(2))) {
    x_diff_bigger = true;
  }
// Simple, dumb movement - suitable for zombies at least
  Point options[5];
  for (int i = 0; i < 5; i++) {
    options[i] = Point(posx, posy);
  }
  if (target_x > posx) {
    if (target_y > posy) {
      options[0] = Point(posx + 1, posy + 1);
      if (x_diff_bigger) {
        options[1] = Point(posx + 1, posy    );
        options[2] = Point(posx    , posy + 1);
        options[3] = Point(posx + 1, posy - 1);
        options[4] = Point(posx - 1, posy + 1);
      } else {
        options[1] = Point(posx    , posy + 1);
        options[2] = Point(posx + 1, posy    );
        options[3] = Point(posx + 1, posy - 1);
        options[4] = Point(posx - 1, posy + 1);
      }
    } else if (target_y < posy) {
      options[0] = Point(posx + 1, posy - 1);
      if (x_diff_bigger) {
        options[1] = Point(posx + 1, posy    );
        options[2] = Point(posx    , posy - 1);
        options[3] = Point(posx + 1, posy + 1);
        options[4] = Point(posx - 1, posy - 1);
      } else {
        options[1] = Point(posx    , posy - 1);
        options[2] = Point(posx + 1, posy    );
        options[3] = Point(posx - 1, posy - 1);
        options[4] = Point(posx + 1, posy + 1);
      }
    } else { // (target_y == posy)
      options[0] = Point(posx + 1, posy    );
      if (one_in(2)) {
        options[1] = Point(posx + 1, posy - 1);
        options[2] = Point(posx + 1, posy + 1);
        options[3] = Point(posx    , posy - 1);
        options[4] = Point(posx    , posy + 1);
      } else {
        options[1] = Point(posx + 1, posy + 1);
        options[2] = Point(posx + 1, posy - 1);
        options[3] = Point(posx    , posy + 1);
        options[4] = Point(posx    , posy - 1);
      }
    }
  } else if (target_x < posx) {
    if (target_y > posy) {
      options[0] = Point(posx - 1, posy + 1);
      if (x_diff_bigger) {
        options[1] = Point(posx - 1, posy    );
        options[2] = Point(posx    , posy + 1);
        options[3] = Point(posx - 1, posy - 1);
        options[4] = Point(posx + 1, posy + 1);
      } else {
        options[1] = Point(posx    , posy + 1);
        options[2] = Point(posx - 1, posy    );
        options[3] = Point(posx - 1, posy - 1);
        options[4] = Point(posx + 1, posy + 1);
      }
    } else if (target_y < posy) {
      options[0] = Point(posx - 1, posy - 1);
      if (x_diff_bigger) {
        options[1] = Point(posx - 1, posy    );
        options[2] = Point(posx    , posy - 1);
        options[3] = Point(posx - 1, posy + 1);
        options[4] = Point(posx + 1, posy - 1);
      } else {
        options[1] = Point(posx    , posy - 1);
        options[2] = Point(posx - 1, posy    );
        options[3] = Point(posx + 1, posy - 1);
        options[4] = Point(posx - 1, posy + 1);
      }
    } else { // (target_y == posy)
      options[0] = Point(posx - 1, posy    );
      if (one_in(2)) {
        options[1] = Point(posx - 1, posy - 1);
        options[2] = Point(posx - 1, posy + 1);
        options[3] = Point(posx    , posy - 1);
        options[4] = Point(posx    , posy + 1);
      } else {
        options[1] = Point(posx - 1, posy + 1);
        options[2] = Point(posx - 1, posy - 1);
        options[3] = Point(posx    , posy + 1);
        options[4] = Point(posx    , posy - 1);
      }
    }
  } else { // (target_x == posx)
    if (target_y > posy) {
      options[0] = Point(posx    , posy + 1);
      if (one_in(2)) {
        options[1] = Point(posx + 1, posy + 1);
        options[2] = Point(posx - 1, posy + 1);
        options[3] = Point(posx + 1, posy    );
        options[4] = Point(posx - 1, posy    );
      } else {
        options[1] = Point(posx - 1, posy + 1);
        options[2] = Point(posx + 1, posy + 1);
        options[3] = Point(posx - 1, posy    );
        options[4] = Point(posx + 1, posy    );
      }
    } else if (target_y < posy) {
      options[0] = Point(posx    , posy - 1);
      if (one_in(2)) {
        options[1] = Point(posx + 1, posy - 1);
        options[2] = Point(posx - 1, posy - 1);
        options[3] = Point(posx + 1, posy    );
        options[4] = Point(posx - 1, posy    );
      } else {
        options[1] = Point(posx - 1, posy - 1);
        options[2] = Point(posx + 1, posy - 1);
        options[3] = Point(posx - 1, posy    );
        options[4] = Point(posx + 1, posy    );
      }
    } else { // (target_y == posy)
      pause();
      return; // Posx and Posy are the same, i.e. we're on top of the target!
    }
  }

// Now that we have an ordered list of favorites, pick one!
// TODO:  Add a "Stumble" flag that occasionally randomly picks, rather than
//        picking the best available.

  for (int i = 0; i < 5; i++) {
    if (can_move_to( GAME.map, options[i].x, options[i].y )) {
      move_to( GAME.map, options[i].x, options[i].y );
      return;
    }
  }
// Couldn't move to any of the spots!  For now, just pause()
// TODO:  Do something better.
  pause();
}

void Monster::wander()
{
  if (wander_duration <= 0) {
    wander_target = Point( posx + rng(-3, 3), posy + rng(-3, 3) );
    wander_duration = 3;
  }
  move_towards(wander_target.x, wander_target.y);
}

void Monster::pause()
{
  action_points = 0;
}

