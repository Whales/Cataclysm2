#include "monster_spawn.h"
#include "monster.h"

Monster_spawn::Monster_spawn()
{
  genus = NULL;
  population = 0;
}

Monster_spawn::~Monster_spawn()
{
}

Monster* Monster_spawn::generate_monster()
{
  if (!genus || population <= 0) {
    return NULL;
  }
  Monster* ret = new Monster();
  ret->set_type( genus->random_member() );
  return ret;
}
