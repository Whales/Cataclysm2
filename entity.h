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

  virtual bool can_move_to(Map* map, int x, int y);
  virtual void move_to(Map* map, int x, int y);

  virtual bool can_sense(Map* map, int x, int y);

  int posx, posy;
  int action_points;
};

class Player : public Entity
{
public:
  Player();
  ~Player();

  virtual std::string get_name();
  virtual glyph get_glyph();
  
  virtual bool can_move_to(Map* map, int x, int y);
private:
  std::string name;
};

#endif
