#include "monster.h"
#include "rng.h"
#include "game.h"
#include "player.h"
#include <sstream>

Monster::Monster()
{
  dead = false;
  type = NULL;
  uid = -1;
  action_points = 0;
}

void Monster::set_type(std::string name)
{
  dead = false;
  type = MONSTER_TYPES.lookup_name(name);
  action_points = 0;
}

Monster::~Monster()
{
}

glyph Monster::top_glyph()
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

void Monster::attack(Entity* entity)
{
  if (!entity) {
    debugmsg("Monster attempted attack() on a null target.");
    return;
  }
  Monster_attack *att = random_attack();
  if (!att) {
    debugmsg("Monster couldn't pick attack!");
    return;
  }

  action_points -= att->speed;

  bool you_see = GAME.player->can_sense(GAME.map, posx, posy);
  if (hit_roll(att->to_hit) < target->dodge_roll()) {
    if (you_see) {
      GAME.add_msg("%s misses %s!", get_name_to_player().c_str(),
                   target->get_name_to_player().c_str());
    }
    return;
  }

  Body_part bp_hit = (target->is_player() ? random_body_part_to_hit() :
                                            BODYPART_NULL);

  int total_damage = 0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    int damage = rng(0, att->damage[i]);
    total_damage += damage;
    entity->take_damage(Damage_type(i), damage, get_name_to_player(),
                        bp_hit);
  }

  if (you_see) {
    std::string damage_str;
    if (target->is_you()) {
      std::stringstream damage_ss;
      damage_ss << "for " << total_damage << " damage";
      damage_str = damage_ss.str();
    }
    if (bp_hit == BODYPART_NULL) {
      GAME.add_msg("%s hits %s %s!", get_name_to_player().c_str(),
                   target->get_name_to_player().c_str(),
                   damage_str.c_str());
    } else {
      GAME.add_msg("%s hits %s %s %s!", get_name_to_player().c_str(),
                   target->get_possessive().c_str(),
                   body_part_name(bp_hit).c_str(), damage_str.c_str());
    }
  }
}

Monster_attack* Monster::random_attack()
{
  if (!type || type->attacks.empty()) {
    return NULL;
  }
  int index = rng(0, type->attacks.size() - 1);
  return &(type->attacks[index]);
}

Body_part Monster::random_body_part_to_hit()
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

Monster_pool::Monster_pool()
{
  next_uid = 0;
}

Monster_pool::~Monster_pool()
{
  for (std::list<Monster*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    delete (*it);
  }
}

void Monster_pool::add_monster(Monster* mon)
{
  if (!mon) {
    debugmsg("Tried to push NULL to Monster_pool");
    return;
  }
  mon->uid = next_uid;
  next_uid++;
  instances.push_back(mon);
  uid_map[mon->uid] = mon;
}

Monster* Monster_pool::lookup_uid(int uid)
{
  if (uid_map.count(uid) == 0) {
    return NULL;
  }
  return uid_map[uid];
}

Monster* Monster_pool::monster_at(int posx, int posy)
{
  for (std::list<Monster*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    if ((*it)->posx == posx && (*it)->posy == posy) {
      return (*it);
    }
  }
  return NULL;
}
