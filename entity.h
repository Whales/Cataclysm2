#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <string>

class Entity
{
public:
  Entity();
  ~Entity();

  virtual std::string get_name();

  int posx, posy;
};

class Player : public Entity
{
public:
  Player();
  ~Player();

  virtual std::string get_name();
private:
  std::string name;
};

#endif
