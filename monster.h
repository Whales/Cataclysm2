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

  virtual glyph get_glyph();
  virtual std::string get_name();
  virtual std::string get_name_to_player();
  virtual std::string get_possessive();
  std::string get_name_indefinite();
  std::string get_name_definite();
  virtual bool is_monster() { return true; }

  virtual bool has_sense(Sense_type sense);

  virtual int get_speed();
  void make_plans();
  virtual void take_turn();

  bool can_attack(Entity* entity);
  virtual Attack base_attack();
  virtual void take_damage(Damage_type type, int damage, std::string reason,
                           Body_part part = BODYPART_NULL);

  void move_towards(Entity* entity);
  void move_towards(int target_x, int target_y);
  void wander();
  void pause();

  Monster_type *type;
  int current_hp;

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
