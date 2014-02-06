#ifndef _ENUM_H_
#define _ENUM_H_

#include <string>
#include <vector> // For get_body_part_list

// TODO: Remove these.  For now I'm not sure how to specify them, but no.
#define SIGHT_DIST 20

enum Sense_type
{
  SENSE_NULL = 0,     // Can't sense anything
  SENSE_SIGHT,        // Blocked by opaque terrain
  SENSE_SOUND,        // Semi-blocked by solid terrain, only shows noise sources
  SENSE_ECHOLOCATION, // Blocked by solid terrain
  SENSE_SMELL,        // Blocked by solid terrain, only shows scent
  SENSE_INFRARED,     // Not blocked, but only works on warm-blooded targets
  SENSE_OMNISCIENT,   // Not blocked by anything!
  SENSE_MAX
};

Sense_type lookup_sense_type(std::string name);
std::string sense_type_name(Sense_type type);

enum Body_part
{
  BODY_PART_NULL = 0,
  BODY_PART_HEAD,
  BODY_PART_TORSO,
  BODY_PART_LEFT_HAND,
  BODY_PART_RIGHT_HAND,
  BODY_PART_LEFT_ARM,
  BODY_PART_RIGHT_ARM,
  BODY_PART_LEFT_FOOT,
  BODY_PART_RIGHT_FOOT,
  BODY_PART_LEFT_LEG,
  BODY_PART_RIGHT_LEG,
  BODY_PART_MAX
};

Body_part lookup_body_part(std::string name);
std::string body_part_name(Body_part part);
// get_body_part_list provides support for "legs" referring to both legs, etc.
std::vector<Body_part> get_body_part_list(std::string name);

enum HP_part
{
  HP_PART_NULL = 0,
  HP_PART_HEAD,
  HP_PART_TORSO,
  HP_PART_LEFT_ARM,
  HP_PART_RIGHT_ARM,
  HP_PART_LEFT_LEG,
  HP_PART_RIGHT_LEG,
  HP_PART_MAX
};

HP_part lookup_HP_part(std::string name);
std::string HP_part_name(HP_part part);
HP_part convert_to_HP(Body_part part);

// TODO: Fire damage? Acid damage? etc etc
enum Damage_type
{
  DAMAGE_NULL = 0,
  DAMAGE_BASH,
  DAMAGE_CUT,
  DAMAGE_PIERCE,
  DAMAGE_FIRE,
  DAMAGE_MAX
};

Damage_type lookup_damage_type(std::string name);
std::string damage_type_name(Damage_type type);

enum Terrain_flag
{
  TF_NULL,
  TF_OPAQUE,        // "opaque" - Blocks sight
  TF_FLOOR,         // "floor" - May be changed by adjacent terrain
  TF_STAIRS_UP,     // "stairs_up" - Can be climbed to gain a Z-level
  TF_STAIRS_DOWN,   // "stairs_down" - Can be climbed to lose a Z-level
  TF_OPEN_SPACE,    // "open_space" - Air.  Shows Z-level below.
  TF_WATER,         // "water" - Swimmable.  Puts out fire.
  TF_FLAMMABLE,     // "flammable" - Consumed by fire.
  TF_MAX
};

#endif
