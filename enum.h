#ifndef _ENUM_H_
#define _ENUM_H_

#include <string>

enum Intel_level
{
  INTEL_NULL = 0,
  INTEL_PLANT,  // Can't do much
  INTEL_ZOMBIE, // Straight-line pathing
  INTEL_ANIMAL, // Limited A* that includes pathing around fields
  INTEL_HUMAN,  // Full A*, use of doors
  INTEL_MAX
};

Intel_level lookup_intel_level(std::string name);
std::string intel_level_name(Intel_level level);

enum Sense_type
{
  SENSE_NULL = 0, // Can't sense anything
  SENSE_SIGHT,    // Blocked by opaque terrain
  SENSE_SOUND,    // Semi-blocked by solid terrain, only shows noise sources
  SENSE_ECHOLOCATION, // Blocked by solid terrain
  SENSE_SMELL,    // Blocked by solid terrain, only shows scent
  SENSE_OMNISCIENT,   // Not blocked by anything!
  SENSE_MAX
};

Sense_type lookup_sense_type(std::string name);
std::string sense_type_name(Sense_type type);

enum Body_part
{
  BODYPART_NULL = 0,
  BODYPART_HEAD,
  BODYPART_TORSO,
  BODYPART_LEFT_ARM,
  BODYPART_RIGHT_ARM,
  BODYPART_LEFT_LEG,
  BODYPART_RIGHT_LEG,
  BODYPART_MAX
};

Body_part lookup_body_part(std::string name);
std::string body_part_name(Body_part part);

// TODO: Fire damage? Acid damage? etc etc
enum Damage_type
{
  DAMAGE_NULL = 0,
  DAMAGE_BASH,
  DAMAGE_CUT,
  DAMAGE_PIERCE,
  DAMAGE_MAX
};

Damage_type lookup_damage_type(std::string name);
std::string damage_type_name(Damage_type type);

enum Terrain_flag
{
  TF_NULL,
  TF_OPAQUE,
  TF_FLOOR,
  TF_MAX
};

#endif
