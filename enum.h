#ifndef _ENUM_H_
#define _ENUM_H_

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


// We use unit64_t here because there's a strong possibility that we'll have
// more than 32 flags.
enum Terrain_flag
{
  TF_NULL,
  TF_OPAQUE,
  TF_MAX
};

#endif
