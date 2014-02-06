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
  for (int i = 0; i < BODY_PART_MAX; i++) {
    Body_part ret = Body_part(i);
    if ( no_caps( body_part_name(ret) ) == name) {
      return ret;
    }
  }
  return BODY_PART_NULL;
}

std::string body_part_name(Body_part part)
{
  switch (part) {
    case BODY_PART_NULL:        return "NULL";
    case BODY_PART_HEAD:        return "head";
    case BODY_PART_TORSO:       return "torso";
    case BODY_PART_LEFT_HAND:   return "left hand";
    case BODY_PART_RIGHT_HAND:  return "right hand";
    case BODY_PART_LEFT_ARM:    return "left arm";
    case BODY_PART_RIGHT_ARM:   return "right arm";
    case BODY_PART_LEFT_FOOT:   return "left foot";
    case BODY_PART_RIGHT_FOOT:  return "right foot";
    case BODY_PART_LEFT_LEG:    return "left leg";
    case BODY_PART_RIGHT_LEG:   return "right leg";
    case BODY_PART_MAX:         return "BUG - BODY_PART_MAX";
    default:                    return "BUG - Unnamed body part";
  }
  return "BUG - Escaped switch";
}

std::vector<Body_part> get_body_part_list(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  std::vector<Body_part> ret;
  if (name == "hands") {
    ret.push_back(BODY_PART_LEFT_HAND);
    ret.push_back(BODY_PART_RIGHT_HAND);
  } else if (name == "feet") {
    ret.push_back(BODY_PART_LEFT_FOOT);
    ret.push_back(BODY_PART_RIGHT_FOOT);
  } else if (name == "arms") {
    ret.push_back(BODY_PART_LEFT_ARM);
    ret.push_back(BODY_PART_RIGHT_ARM);
  } else if (name == "legs") {
    ret.push_back(BODY_PART_LEFT_LEG);
    ret.push_back(BODY_PART_RIGHT_LEG);
  } else if (name == "all") {
    for (int i = 1; i < BODY_PART_MAX; i++) {
      ret.push_back( Body_part(i) );
    }
  } else {
    Body_part bp = lookup_body_part(name);
    if (bp != BODY_PART_NULL) {
      ret.push_back(bp);
    }
  }
  return ret;
}

HP_part lookup_HP_part(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < HP_PART_MAX; i++) {
    HP_part ret = HP_part(i);
    if ( no_caps(HP_part_name(ret)) == name ) {
      return ret;
    }
  }
  return HP_PART_NULL;
}

std::string HP_part_name(HP_part part)
{
  switch (part) {
    case HP_PART_NULL:      return "NULL";
    case HP_PART_HEAD:      return "Head";
    case HP_PART_TORSO:     return "Torso";
    case HP_PART_LEFT_ARM:  return "Left Arm";
    case HP_PART_RIGHT_ARM: return "Right Arm";
    case HP_PART_LEFT_LEG:  return "Left Leg";
    case HP_PART_RIGHT_LEG: return "Right Leg";
    case HP_PART_MAX:       return "BUG - HP_PART_MAX";
    default:                return "BUG - Unnamed HP_part";
  }
  return "BUG - Escaped HP_part_name switch";
}

HP_part convert_to_HP(Body_part part)
{
  switch (part) {
    case BODY_PART_HEAD:        return HP_PART_HEAD;
    case BODY_PART_TORSO:       return HP_PART_TORSO;
    case BODY_PART_LEFT_HAND:
    case BODY_PART_LEFT_ARM:    return HP_PART_LEFT_ARM;
    case BODY_PART_RIGHT_HAND:
    case BODY_PART_RIGHT_ARM:   return HP_PART_RIGHT_ARM;
    case BODY_PART_LEFT_FOOT:
    case BODY_PART_LEFT_LEG:    return HP_PART_LEFT_LEG;
    case BODY_PART_RIGHT_FOOT:
    case BODY_PART_RIGHT_LEG:   return HP_PART_RIGHT_LEG;
    default:                    return HP_PART_NULL;
  }
  return HP_PART_NULL;
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
