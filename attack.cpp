#include "attack.h"

Attack::Attack()
{
  verb = "hits";
  weight =  10;
  speed  = 100;
  to_hit =   0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] = 0;
  }
}


