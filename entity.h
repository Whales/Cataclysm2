#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <string>
#include "glyph.h"
#include "map.h"
#include "item.h"

class Entity
{
public:
  Entity();
  virtual ~Entity();

  virtual std::string get_name();
  virtual std::string get_name_to_player();
  virtual std::string get_possessive();
  virtual glyph get_glyph();

  virtual bool is_player()  { return false; };
  virtual bool is_monster() { return false; };
  virtual bool is_you()     { return false; }; // As in THE player

  virtual void gain_action_points();
  virtual int get_speed();

  virtual bool can_move_to(Map* map, int x, int y);
  virtual void move_to(Map* map, int x, int y);
  virtual void pause();

// Combat functions
  virtual void attack(Entity* target);
  virtual int hit_roll(int bonus);
  virtual int dodge_roll();
  virtual void take_damage(Damage_type type, int damage, std::string reason,
                           Body_part part);

  virtual bool can_sense(Map* map, int x, int y);

  int posx, posy;
  int action_points;

  Item weapon;
  std::vector<Item> inventory;
};
#endif
