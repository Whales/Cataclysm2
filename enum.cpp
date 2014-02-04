#include "enum.h"
#include "stringfunc.h"

Sense_type lookup_sense_type(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < SENSE_MAX; i++) {
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

Body_part lookup_body_part(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < BODYPART_MAX; i++) {
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

Damage_type lookup_damage_type(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < DAMAGE_MAX; i++) {
    Damage_type ret = Damage_type(i);
    if (name == no_caps( damage_type_name(ret) ) ) {
      return ret;
    }
  }
  return DAMAGE_NULL;
}

std::string damage_type_name(Damage_type type)
{
  switch (type) {
    case DAMAGE_NULL:   return "NULL";
    case DAMAGE_BASH:   return "bash";
    case DAMAGE_CUT:    return "cut";
    case DAMAGE_PIERCE: return "pierce";
    case DAMAGE_FIRE:   return "fire";
    case DAMAGE_MAX:    return "BUG - DAMAGE_MAX";
    default:            return "Unnamed Damage_type";
  }
  return "BUG - Escaped switch";
}
