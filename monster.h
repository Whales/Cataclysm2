#ifndef _MONSTER_H_
#define _MONSTER_H_

#include <string>
#include <list>
#include <map>
#include "entity.h"
#include "glyph.h"
#include "geometry.h"
#include "enum.h"

class Monster : public Entity
{
public:
  Monster();
  ~Monster();

  void set_type(std::string name);

  virtual glyph top_glyph();
  virtual std::string get_name();
  virtual std::string get_name_to_player();
  virtual std::string get_possessive();
  std::string get_name_indefinite();
  std::string get_name_definite();
  virtual bool is_monster() { return true; };

  bool has_sense(Sense_type sense);

  void gain_action_points();
  void make_plans();
  void take_turn();

  bool can_attack(Entity* entity);
  virtual void attack(Entity* entity);
  Monster_attack* random_attack();
  Body_part random_body_part_to_hit();

  void move_towards(Entity* entity);
  void move_towards(int target_x, int target_y);
  void wander();
  void pause();

  Monster_type *type;
  int uid;
  bool dead;

private:

  Entity* target;
  Point wander_target;
  int wander_duration;
  
};

/* For now, Monster_pool does NOT include a map which uses location as a key.
 * In order for this map to be useful, we'd have to update it every turn, which
 * means it' probably be more trouble than it's worth, except when the map is
 * being called several times per turn.  We'd also have to update it after
 * every monster moves, which is a lot.
 * This means that monster_at() has to iterate over all monsters, which is
 * potentially slow, but what can you do.
 */

class Monster_pool
{
public:
  Monster_pool();
  ~Monster_pool();

  void add_monster(Monster* mon);

  Monster* lookup_uid(int uid);
  Monster* monster_at(int posx, int posy);

  std::list<Monster*> instances;
private:
  std::map<int,Monster*> uid_map;
  int next_uid;
};
  

#endif
