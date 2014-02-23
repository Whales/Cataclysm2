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
  BODY_PART_EYES,
  BODY_PART_MOUTH,
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

// Terrain_flag has its lookup and name functions in terrain.cpp
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
  TF_CONTAINER,     // "container" - Can hold items despite move_cost of 0
  TF_MAX
};

// Item_action & Item_flag have their lookup and name functions defd in item.cpp
enum Item_action
{
  IACT_NULL = 0,
  IACT_WIELD, // Wield as a wearpon
  IACT_WEAR,  // Wear - only applies for clothing
  IACT_DROP,  // Drop on the ground
  IACT_EAT,   // Eat - only applies for food
  IACT_APPLY, // Apply as in a tool
  IACT_EMPTY, // As in emptying a container - only applies for containers
  IACT_MAX
};

enum Item_flag
{
  ITEM_FLAG_NULL = 0,
  ITEM_FLAG_LIQUID,     // "liquid" - Puts out fires, needs a container
  ITEM_FLAG_FLAMMABLE,  // "flammable" - Consumed by fires
  ITEM_FLAG_PLURAL,     // "plural" - indefinite article is "some," not "a"
  ITEM_FLAG_CONSTANT,   // "constant_volume_weight" - doesn't use usual food v/w
  ITEM_FLAG_MAX
};

enum Stat_id
{
  STAT_NULL = 0,
  STAT_STRENGTH,
  STAT_DEXTERITY,
  STAT_INTELLIGENCE,
  STAT_PERCEPTION,
  STAT_MAX
};

Stat_id lookup_stat_id(std::string name);
std::string stat_id_name(Stat_id id);
std::string stat_id_short_name(Stat_id id);

// These are used, for now, in signal_handlers
// TODO: Expand this?  It's simple right now...
enum Math_operator
{
  MATH_NULL = 0,
  MATH_MULTIPLY,
  MATH_GREATER_THAN,
  MATH_GREATER_THAN_OR_EQUAL_TO,
  MATH_LESS_THAN,
  MATH_LESS_THAN_OR_EQUAL_TO,
  MATH_EQUAL_TO,
  MATH_MAX
};

Math_operator lookup_math_operator(std::string name);
std::string math_operator_name(Math_operator op);

#endif
