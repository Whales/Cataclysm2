#ifndef _MONSTER_H_
#define _MONSTER_H_

#include "monster_type.h"
#include "entity.h"
#include "glyph.h"
#include "geometry.h"
#include "enum.h"
#include <string>
#include <list>
#include <map>

class Monster : public Entity
{
public:
  Monster();
  ~Monster();

  void set_type(std::string name);
  void set_type(Monster_type* newtype);

  virtual glyph get_glyph();
  virtual std::string get_data_name();
  virtual std::string get_name();
  virtual std::string get_name_to_player();
  virtual std::string get_possessive();
  std::string get_name_indefinite();
  std::string get_name_definite();
  virtual bool is_monster() { return true; }

  virtual bool has_sense(Sense_type sense);
  virtual Entity_AI   get_AI();
  virtual int get_genus_uid();

  virtual int get_speed();
  void make_plans();
  virtual bool try_goal(AI_goal goal);
  virtual bool pick_attack_victim();
  virtual bool pick_flee_target();
  virtual void take_turn();

  virtual bool can_sense(Entity* entity);
  bool can_attack(Entity* entity);
  virtual Attack base_attack();
  virtual void take_damage(Damage_type type, int damage, std::string reason,
                           Body_part part = BODYPART_NULL);

  void move_towards(Entity* entity);
  void move_towards(int target_x, int target_y);
  void move_towards(Tripoint target);
  void wander();
  void pause();

  Monster_type *type;
  int current_hp;

private:
  Entity* entity_target;
  Tripoint wander_target;
  int wander_duration;
};

#endif
