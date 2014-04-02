#include "trait.h"
#include <string>

Trait_id lookup_trait_id(std::string name)
{
  name = no_caps( trim( name ) );
  for (int i = 0; i < TRAIT_MAX; i++) {
    Trait_id ret = Trait_id(i);
    if (name == no_caps( trait_id_name(ret) ) ) {
      return ret;
    }
  }
  return TRAIT_NULL;
}

std::string trait_id_name(Trait_id trait)
{
  switch (trait) {

    case TRAIT_NULL:
      return "NULL";

    case TRAIT_FLEET:
      return "Fleet-Footed";

    case TRAIT_QUICK:
      return "Quick";

    case TRAIT_LIGHT_EATER:
      return "Light Eater";

    case TRAIT_CAMEL:
      return "Camel";

    case TRAIT_NIGHT_VISION:
      return "Night Vision";

    case TRAIT_DURABLE:
      return "Durable";

    case TRAIT_PACKMULE:
      return "Packmule";

    case TRAIT_ROBUST:
      return "Robust";

    case TRAIT_MAX_GOOD:
      return "BUG - TRAIT_MAX_GOOD";

    case TRAIT_LIGHTWEIGHT:
      return "Lightweight";

    case TRAIT_MAX_NEUTRAL:
      return "BUG - TRAIT_MAX_NEUTRAL";

    case TRAIT_MYOPIC:
      return "Myopic";

    case TRAIT_BAD_BACK:
      return "Bad Back";

    case TRAIT_BAD_HEARING:
      return "Bad Hearing";

    case TRAIT_SMELLY:
      return "Smelly";

    case TRAIT_MAX_BAD:
      return "BUG - TRAIT_MAX_BAD";

    case TRAIT_MAX:
      return "BUG - TRAIT_MAX";

    default:
      return "BUG - Unnamed Trait_id";
  }

  return "BUG - Escaped Trait_id_name() switch!";
}
