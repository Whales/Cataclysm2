#ifndef _TRAIT_H_
#define _TRAIT_H_

#include <string>

enum Trait_id
{
  TRAIT_NULL = 0,
  TRAIT_FLEET,        // Terrain movement cost of 100 is effectively 90
  TRAIT_QUICK,        // +5 AP each turn
  TRAIT_LIGHT_EATER,  // Hunger increases more slowly
  TRAIT_CAMEL,        // Thirst increases more slowly
  TRAIT_NIGHT_VISION, // Sight radius is better at night
  TRAIT_DURABLE,      // +??% HP, overall constitution boost (eg disease resist)
  TRAIT_PACKMULE,     // +20% increase to volume capacity
  TRAIT_ROBUST,       // Mutations are more likely to be good

  TRAIT_MAX_GOOD,     // Splits good traits from neutral traits

  TRAIT_LIGHTWEIGHT,  // Increased duration of drug effects, both good & bad

  TRAIT_MAX_NEUTRAL,  // Splits neutral traits from bad traits

  TRAIT_MYOPIC,       // Limited sight range w/o glasses; start w/ glasses
  TRAIT_BAD_BACK,     // -20% weight capacity
  TRAIT_BAD_HEARING,  // Volume of sounds is decreased by 50%
  TRAIT_SMELLY,       // Increased smell radius

  TRAIT_MAX_BAD,      // Splits bad traits from mutations

  TRAIT_MAX           // ALWAYS last in this list
};

Trait_id lookup_trait_id(std::string name);
std::string trait_id_name(Trait_id trait);
std::string trait_description(Trait_id trait);

#endif
