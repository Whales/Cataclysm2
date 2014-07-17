#ifndef _MONSTER_EFFECT_H_
#define _MONSTER_EFFECT_H_

enum Monster_effect_type
{
  MON_EFFECT_NULL = 0,  // nuffin
  MON_EFFECT_SUMMON,    // create one or more named monsters
  MON_EFFECT_SIGNAL,    // apply signal to nearby terrain
  MON_EFFECT_MAX
};

#endif
