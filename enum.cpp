#include "enum.h"
#include "stringfunc.h"

Sense_type lookup_sense_type(std::string name)
{
  name = no_caps(name);
  for (int i = 1; i < SENSE_MAX; i++) {
    Sense_type ret = Sense_type(i);
    if ( no_caps( sense_type_name(ret) ) == name) {
      return ret;
    }
  }
  return SENSE_NULL;
}

std::string sense_type_name(Sense_type type)
{
  switch (type) {
    case SENSE_NULL:          return "NULL Sense";
    case SENSE_SIGHT:         return "sight";
    case SENSE_SOUND:         return "sound";
    case SENSE_ECHOLOCATION:  return "echolocation";
    case SENSE_SMELL:         return "smell";
    case SENSE_OMNISCIENT:    return "omniscient";
    case SENSE_MAX:           return "BUG - SENSE_MAX";
  }
  return "Unknown Sense";
}
