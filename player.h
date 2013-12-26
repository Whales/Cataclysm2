#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <string>
#include "entity.h"
#include "map.h"
#include "enum.h"

class Player : public Entity
{
public:
  Player();
  ~Player();

  virtual std::string get_name();
  virtual std::string get_name_to_player();
  virtual std::string get_possessive();
  virtual glyph get_glyph();

  virtual bool is_player()  { return true; };
  virtual bool is_you()     { return true; }; // TODO: No?
  
  virtual bool can_move_to(Map* map, int x, int y);

  virtual void take_damage(Damage_type type, int damage, std::string reason,
                           Body_part part);

  std::string hp_text(Body_part part);

  int current_hp[BODYPART_MAX];
  int max_hp[BODYPART_MAX];

private:
  std::string name;
};

#endif
