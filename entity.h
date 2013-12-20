#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <string>
#include "glyph.h"

class Entity
{
public:
  Entity();
  virtual ~Entity();

  virtual std::string get_name();
  virtual glyph get_glyph();

  int posx, posy;
};

class Player : public Entity
{
public:
  Player();
  ~Player();

  virtual std::string get_name();
  virtual glyph get_glyph();
private:
  std::string name;
};

#endif
