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
  Monster(std::string name);
  Monster(Monster_type* newtype);
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

  virtual void die();

  virtual bool has_sense(Sense_type sense);
  virtual Entity_AI   get_AI();
  virtual int get_genus_uid();

  virtual int get_speed();

  //virtual bool is_enemy(Entity* ent);
  void make_plans();
  virtual bool try_goal(AI_goal goal);
  virtual bool pick_attack_victim();
  virtual bool pick_flee_target();
  virtual std::vector<Ranged_attack> get_ranged_attacks();
  virtual void take_turn();
  void make_sound();
  void use_ability();

  //virtual bool can_sense(Entity* entity);
  virtual Attack base_attack();
  virtual int hit_roll(int bonus);
  virtual int dodge_roll();

  virtual void take_damage(Damage_type damtype, int damage, std::string reason,
                           Body_part part = BODY_PART_NULL);

  virtual void take_damage_no_armor(Damage_type damtype, int damage,
                                    std::string reason,
                                    Body_part part = BODY_PART_NULL);

  virtual void absorb_damage(Damage_type damtype, int &damage, Body_part part);
  virtual void heal_damage(int damage, HP_part part = HP_PART_NULL);
  virtual int  get_armor(Damage_type damtype, Body_part part = BODY_PART_NULL);

  void move_towards(Entity* entity);
  void move_towards(int target_x, int target_y);
  void move_towards(Tripoint target);
  void wander();
  void pause();

  //void shift(int shiftx, int shifty); // For map shifting

  Monster_type *type;
  int current_hp, max_hp;

};

#endif
