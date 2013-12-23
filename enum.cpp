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
    default:                  return "BUG - Unnamed Sense";
  }
  return "BUG - Escaped switch";
}

Body_part lookup_bory_part(std::string name)
{
  name = no_caps(name);
  for (int i = 1; i < BODYPART_MAX; i++) {
    Body_part ret = Body_part(i);
    if ( no_caps( body_part_name(ret) ) == name) {
      return ret;
    }
  }
  return BODYPART_NULL;
}

std::string body_part_name(Body_part part)
{
  switch (part) {
    case BODYPART_NULL:       return "NULL";
    case BODYPART_HEAD:       return "head";
    case BODYPART_TORSO:      return "torso";
    case BODYPART_LEFT_ARM:   return "left arm";
    case BODYPART_RIGHT_ARM:  return "right arm";
    case BODYPART_LEFT_LEG:   return "left leg";
    case BODYPART_RIGHT_LEG:  return "right leg";
    case BODYPART_MAX:        return "BUG - BODYPART_MAX";
    default:                  return "BUG - Unnamed body part";
  }
  return "BUG - Escaped switch";
}
