#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <string>
#include "glyph.h"
#include "map.h"

class Entity
{
public:
  Entity();
  virtual ~Entity();

  virtual std::string get_name();
  virtual glyph get_glyph();

  virtual bool is_player()  { return false; };
  virtual bool is_monster() { return false; };

  virtual bool can_move_to(Map* map, int x, int y);
  virtual void move_to(Map* map, int x, int y);

// Combat functions
  virtual void attack(Entity* target);
  virtual int  max_damage(Damage_type type = DAMAGE_BASH);

  virtual bool can_sense(Map* map, int x, int y);

  int posx, posy;
  int action_points;
};
#endif
