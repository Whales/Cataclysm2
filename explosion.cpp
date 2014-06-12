#include "explosion.h"
#include "window.h" // For debugmsg
#include "stringfunc.h" // For no_caps() and trim()
#include "entity.h"
#include "game.h"
#include "damage_set.h" // For damaging terrain
#include "attack.h" // For shrapnel Ranged_attack
#include "rng.h"
#include <sstream>  // For setting the creator of any fields we create!

Explosion::Explosion()
{
  radius = Dice(0, 0, 1);
  force  = Dice(0, 0, 1);

  field_chance = 0;
}

Explosion::~Explosion()
{
}

bool Explosion::load_data(std::istream& data, std::string owner_name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "radius:") {
      if (!radius.load_data(data, owner_name + " radius")) {
        return false;
      }

    } else if (ident == "force:") {
      if (!force.load_data(data, owner_name + " force")) {
        return false;
      }

    } else if (ident == "shrapnel_count:") {
      if (!shrapnel_count.load_data(data, owner_name + " shrapnel count")) {
        return false;
      }

    } else if (ident == "shrapnel_damage:") {
      if (!shrapnel_damage.load_data(data, owner_name + " shrapnel damage")) {
        return false;
      }

    } else if (ident == "field:" || ident == "field_name:") {
      std::getline(data, field_name);
      field_name = no_caps(field_name);
      field_name = trim(field_name);
      if (field_name.empty()) {
        debugmsg("Empty explosion field name (%s)", owner_name.c_str());
        return false;
      }

    } else if (ident == "field_chance:") {
      data >> field_chance;
      if (field_chance < 1) {
        debugmsg("Correcting field_chance of %d to 1. (%s)", field_chance,
                 owner_name.c_str());
        field_chance = 1;
      } else if (field_chance > 100) {
        debugmsg("Correcting field_chance of %d to 100. (%s)", field_chance,
                 owner_name.c_str());
        field_chance = 100;
      }
      std::getline(data, junk);

    } else if (ident == "field_duration:") {
      if (!field_duration.load_data(data, owner_name + " field duration")) {
        return false;
      }

    } else if (ident != "done") {
      debugmsg("Unknown Explosion property '%s' (%s)", ident.c_str(),
               owner_name.c_str());
      return false;
    }
  }
  return true;
}

// TODO: Z axis (or not?)
void Explosion::explode(Tripoint epicenter)
{
  int damage = force.roll();
  if (damage <= 0) {
    return; // No damage at all!
  }
  int rad = radius.roll();
  if (rad < 0) {
    return; // No radius!
  }

// Lookup the field type before anything else
  Field_type* ftype = NULL;
  if (!field_name.empty()) {
    ftype = FIELDS.lookup_name(field_name);
    if (ftype == NULL) {  // Failed to look it up!
      debugmsg("Explosion tried to spawn field '%s' but it doesn't exist.",
               field_name.c_str());
// We could return here, but we don't. We can still do the rest of the explosion
    }
  }

// First, do the basic concussive force and fields.
  for (int x = epicenter.x - rad; x <= epicenter.x + rad; x++) {
    for (int y = epicenter.y - rad; y <= epicenter.y + rad; y++) {
      Tripoint pos(x, y, epicenter.z);
      int distance = rl_dist(epicenter, pos);
// Scale damage with distance.
      int dam = (damage * (rad - distance + 1)) / (rad + 1);
// Damage any entity there.
      Entity* ent = GAME.entities.entity_at(pos);
      Damage_set dam_set;
      dam_set.set_damage(DAMAGE_BASH, dam);
      if (ent) {
        ent->take_damage_everywhere(DAMAGE_BASH, dam, reason);
      }
// TODO: Explosive items explode recursively?
// Damage the terrain there.  This may cause an explosion!
      GAME.map->damage(pos, dam_set);
// Plant a field, perhaps?
      if (ftype && rng(1, 100) <= field_chance) {
        Field field_placed(ftype);
        field_placed.set_duration( field_duration.roll() );
        field_placed.adjust_level(); // Set level, based on duration
        std::stringstream field_creator;
        field_creator << "an explosion";
        if (!reason.empty()) {
          field_creator << ", created by " << reason;
        }
        field_placed.creator = field_creator.str();
        GAME.map->add_field(field_placed, pos);
      }
    } // y-loop
  } // x-loop

// Next, do shrapnel.
  int num_shrapnel = shrapnel_count.roll();
  for (int i = 0; i < num_shrapnel; i++) {
    Tripoint shrapnel_target;
    shrapnel_target.x = rng(epicenter.x - rad, epicenter.x + rad);
    shrapnel_target.y = rng(epicenter.y - rad, epicenter.y + rad);
    shrapnel_target.z = epicenter.z;
// Set up a Ranged_attack for the shrapnel
// TODO: Actually throw an item?
    Ranged_attack shrapnel_attack;
    shrapnel_attack.type = RANGED_ATT_OTHER;
    shrapnel_attack.range = rad;
    shrapnel_attack.variance = Dice(1, 6, 0); // Do we even need this?
// TODO: Should shrapnel always pierce?  Maybe it should cut?
    shrapnel_attack.damage[DAMAGE_PIERCE] = shrapnel_damage.roll();
    GAME.launch_projectile(shrapnel_attack, epicenter, shrapnel_target);
  }
}
