#include "monster.h"
#include "rng.h"
#include "game.h"

Monster::Monster(Monster_type *T)
{
  dead = false;
  type = T;
  uid = -1;
  if (type) {
    action_points = type->speed;
  } else {
    action_points = 0;
  }
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

void Monster::make_plans()
{
  Player *player = GAME.player;
  Map *map = GAME.map;
// TODO: Support different senses
// TODO: Support non-aggressive monsters
  if (can_sense(map, player->posx, player->posy)) {
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
  debugmsg("Monster attack()");
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
