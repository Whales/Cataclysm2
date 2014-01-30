#ifndef _MONSTER_SPAWN_H_
#define _MONSTER_SPAWN_H_

class Monster;
class Monster_genus;

struct Monster_spawn
{
  Monster_spawn();
  ~Monster_spawn();

  Monster_genus* genus;
  int population;
// TODO: Other values.  Mobilization?  Status - on alert, passive, aggressive?

  Monster* generate_monster();
};

#endif
