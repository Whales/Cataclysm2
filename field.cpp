#include "field.h"
#include "window.h"     // For debugmsg
#include "stringfunc.h" // For no_caps and trim
#include "terrain.h"    // For Terrain_flag
#include "globals.h"    // For TERRAIN
#include <sstream>

bool Field_terrain_fuel::load_data(std::istream& data,
                                       std::string owner_name)
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

    } else if (ident == "terrain_flag:") {
      std::string flagname;
      std::getline(data, flagname);
      flagname = trim(flagname);
      Terrain_flag tf = lookup_terrain_flag(flagname);
      if (tf == TF_NULL) {
        debugmsg("Unknown terrain flag '%s' (%s)", flagname.c_str(),
                 owner_name.c_str());
        return false;
      }
      flag = tf;

    } else if (ident == "fuel:") {
      data >> fuel;
      std::getline(data, junk);

    } else if (ident == "damage:") {
      data >> damage;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown Field_terrain_fuel property '%s' (%s)", ident.c_str(),
               owner_name.c_str());
      return false;
    }
  }
  return true;
}
  
bool Field_item_fuel::load_data(std::istream& data,
                                       std::string owner_name)
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

    } else if (ident == "item_flag:") {
      std::string flagname;
      std::getline(data, flagname);
      flagname = trim(flagname);
      Item_flag itf = lookup_item_flag(flagname);
      if (itf == ITEM_FLAG_NULL) {
        debugmsg("Unknown item flag '%s' (%s)", flagname.c_str(),
                 owner_name.c_str());
        return false;
      }
      flag = itf;

    } else if (ident == "fuel:") {
      data >> fuel;
      std::getline(data, junk);

    } else if (ident == "damage:") {
      data >> damage;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown Field_item_fuel property '%s' (%s)", ident.c_str(),
               owner_name.c_str());
      return false;
    }
  }
  return true;
}

Field_level::Field_level()
{
  duration = 0;
  duration_lost_without_fuel = 0;
  for (int i = 0; i < TF_MAX; i++) {
    terrain_flags.push_back(false);
  }
}

Field_level::~Field_level()
{
}

bool Field_level::load_data(std::istream& data, std::string owner_name)
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

    } else if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);

    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);

    } else if (ident == "duration:") {
      data >> duration;
      std::getline(data, junk);

    } else if (ident == "duration_lost_without_fuel:") {
      data >> duration_lost_without_fuel;
      std::getline(data, junk);

    } else if (ident == "verb:") {
      std::getline(data, verb);
      verb = trim(verb);

    } else if (ident == "damages:") {
      std::string body_part_line;
      std::getline(data, body_part_line);
      std::istringstream body_part_data(body_part_line);
      std::string body_part_name;
      while (body_part_data >> body_part_name) {
        if (body_part_name == "arms") {
          body_parts_hit.push_back(BODYPART_LEFT_ARM);
          body_parts_hit.push_back(BODYPART_RIGHT_ARM);
        } else if (body_part_name == "legs") {
          body_parts_hit.push_back(BODYPART_LEFT_LEG);
          body_parts_hit.push_back(BODYPART_RIGHT_LEG);
        } else {
          Body_part bp = lookup_body_part( body_part_name );
          if (bp == BODYPART_NULL) {
            debugmsg("Unknown body part '%s' (%s:%s)", body_part_name.c_str(),
                     owner_name.c_str(), name.c_str());
            return false;
          }
          body_parts_hit.push_back(bp);
        }
      }

    } else if (ident == "flags:") {
      std::string flag_line;
      std::getline(data, flag_line);
      std::istringstream flag_data(flag_line);
      std::string flag_name;
      while (flag_data >> flag_name) {
        Terrain_flag tf = lookup_terrain_flag(flag_name);
        if (tf == TF_NULL) {
          debugmsg("Unknown terrain flag '%s' (%s:%s)", flag_name.c_str(),
                   owner_name.c_str(), name.c_str());
          return false;
        }
        terrain_flags[tf] = true;
      }

    } else if (ident != "done") {
/* Check if it's a damage type; TODO: make this less ugly (duplicated in
 *                                    attack.cpp!)
 * Damage_set::load_data() maybe?
 */
      std::string damage_name = ident;
      size_t colon = damage_name.find(':');
      if (colon != std::string::npos) {
        damage_name = damage_name.substr(0, colon);
      }
      Damage_type type = lookup_damage_type(damage_name);
      if (type == DAMAGE_NULL) {
        debugmsg("Unknown Field_level property '%s' (%s:%s)", ident.c_str(),
                 owner_name.c_str(), name.c_str());
        return false;
      } else {
        int tmpdam;
        data >> tmpdam;
        damage.set_damage(type, tmpdam);
      }
    }

  }
  return true;
}

std::string Field_level::get_name()
{
  return name;
}

bool Field_level::has_flag(Terrain_flag tf)
{
  return terrain_flags[tf];
}

Field_type::Field_type()
{
  uid = -1;
  spread_chance = 0;
}

Field_type::~Field_type()
{
  for (int i = 0; i < levels.size(); i++) {
    delete (levels[i]);
  }
}

std::string Field_type::get_data_name()
{
  return name;
}

std::string Field_type::get_name()
{
  if (display_name.empty()) {
    return name;
  }
  return display_name;
}

std::string Field_type::get_level_name(int level)
{
  if (level < 0 || level >= levels.size()) {
    std::stringstream ret;
    ret << "ERROR: Out-of-bounds " << get_name() << " [" << level << "]";
    return ret.str();
  }

  return levels[level]->get_name();
}

Field_level* Field_type::get_level(int level)
{
  if (level < 0 || level >= levels.size()) {
    return NULL;
  }
  return levels[level];
}

int Field_type::duration_needed_to_reach_level(int level)
{
  if (level <= 0 || level >= levels.size()) {
    return 999999;
  }
  return levels[level - 1]->duration + levels[level]->duration;
}

int Field_type::get_uid()
{
  return uid;
}

bool Field_type::load_data(std::istream& data)
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

    } else if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);

    } else if (ident == "display_name:") {
      std::getline(data, display_name);
      display_name = trim(display_name);

    } else if (ident == "spread_chance:") {
      data >> spread_chance;
      if (spread_chance < 0 || spread_chance > 100) {
        debugmsg("Bad spread_chance %d (%s)", spread_chance, name.c_str());
        return false;
      }
      std::getline(data, junk);

    } else if (ident == "spread_cost:") {
      data >> spread_cost;
      if (spread_cost < 0 || spread_cost > 100) {
        debugmsg("Bad spread_cost %d (%s)", spread_cost, name.c_str());
        return false;
      }
      std::getline(data, junk);

    } else if (ident == "output_chance:") {
      data >> output_chance;
      if (output_chance < 0 || output_chance > 100) {
        debugmsg("Bad output_chance %d (%s)", output_chance, name.c_str());
        return false;
      }
      std::getline(data, junk);

    } else if (ident == "output_cost:") {
      data >> output_cost;
      if (output_cost < 0 || output_cost > 100) {
        debugmsg("Bad output_cost %d (%s)", output_cost, name.c_str());
        return false;
      }
      std::getline(data, junk);

    } else if (ident == "output_type:") {
      std::getline(data, output_type);
      output_type = trim(output_type);

    } else if (ident == "terrain_fuel:") {
      Field_terrain_fuel tmpmod;
      if (tmpmod.load_data(data, name)) {
        terrain_fuels.push_back(tmpmod);
      }

    } else if (ident == "item_fuel:") {
      Field_item_fuel tmpmod;
      if (tmpmod.load_data(data, name)) {
        item_fuels.push_back(tmpmod);
      }


    } else if (ident == "level:") {
      Field_level* tmp_level = new Field_level;
      if (!tmp_level->load_data(data, name)) {
        delete tmp_level;
        return false;
      }
      levels.push_back(tmp_level);

    } else if (ident != "done") {
      debugmsg("Unknown Field_type property: '%s' (%s)", ident.c_str(),
               name.c_str());
      return false;
    }
  }
  return true;
}

Field::Field(Field_type* T, int L, std::string C)
{
  type = T;
  level = L;
  creator = C;
  duration = 0;
  if (type) {
    Field_level* lev = type->get_level(level);
    if (lev) {
      duration = lev->duration;
    }
  }
}

Field::~Field()
{
}

std::string Field::get_name()
{
  if (!type) {
    return "BUG - Typeless field";
  }
  Field_level* lev = type->get_level(level);
  if (!lev) {
    return "BUG - Invalid field level";
  }
  return lev->name;
}

std::string Field::get_full_name()
{
  std::stringstream ret;
  ret << get_name();
  if (!creator.empty()) {
    ret << " (created by " << creator << ")";
  }
  return ret.str();
}

void Field::hit_entity(Entity* entity)
{
  if (!entity || !type) {
    return;
  }
  Field_level* lev = type->get_level(level);
  if (!lev) {
    return;
  }

  std::list<Body_part>* bp_hit = &(lev->body_parts_hit);
  if (bp_hit->empty()) {
    return;
  }
  for (std::list<Body_part>::iterator it = bp_hit->begin();
       it != bp_hit->end();
       it++) {
    entity->take_damage(lev->damage, get_full_name(), (*it));
  }
// TODO: Status effects etc.
}

/* Field::process handles its per-turn effects:
 *  Lose 1 duration
 *  Consume fuel, if any is available
 *  Lose extra duration, if our type calls for it, if no fuel was consumed
 *  Change level, if appropriate
 */
void Field::process(Tile* tile_here)
{
  if (!tile_here) {
    debugmsg("Field::process() called without tile information!");
    return;
  }
  if (!type) {
    debugmsg("Field::process() called for a Field without any type!");
    return;
  }

  Field_level* level_data = type->get_level(level);

  duration--;
  bool found_fuel = false;
// Check for terrain fuel/extinguishers
  for (std::list<Field_terrain_fuel>::iterator it = type->terrain_fuels.begin();
       it != type->terrain_fuels.end();
       it++) {
    Field_terrain_fuel fuel = (*it);
    if (tile_here->has_flag(fuel.flag)) {
      found_fuel = true;
      int fuel_gained = fuel.fuel;
      if (tile_here->hp < fuel.damage) {
        double fuel_percent = tile_here->hp / fuel.damage;
        fuel_gained = int( double(fuel.fuel) * fuel_percent );
      }
      duration += fuel_gained;
// TODO: Assign a damage type to Field_terrain_fuel?
      tile_here->damage(DAMAGE_NULL, fuel.damage);
    }
  }

// Check for item fuel/extinguishers
  for (std::list<Field_item_fuel>::iterator it = type->item_fuels.begin();
       it != type->item_fuels.end();
       it++) {
    Field_item_fuel fuel = (*it);
    for (int n = 0; n < tile_here->items.size(); n++) {
      found_fuel = true;
      Item* it = &(tile_here->items[n]);
      if (it->has_flag(fuel.flag)) {
        duration += fuel.fuel;
        if (it->damage(fuel.damage)) { // Item was destroyed
          tile_here->items.erase( tile_here->items.begin() + n );
          n--;
        }
      }
    }
  }

// Lose extra duration if we didn't find fuel
  if (!found_fuel && level_data) {
    duration -= level_data->duration_lost_without_fuel;
  }

// Change our level, if appropriate!
  while (duration >= type->duration_needed_to_reach_level(level + 1)) {
    gain_level();
  }

// TODO: Output extra stuff (output_type from type)
}

void Field::gain_level()
{
  if (!type) {
    return;
  }
  Field_level* lev = type->get_level(level);
  if (!lev) {
    return;
  }
  duration -= lev->duration;
  level++;
}

void Field::lose_level()
{
  if (level == 0) {
    dead = true;
  }
  level--;
  if (!type) {
    return;
  }
  Field_level* lev = type->get_level(level);
  if (!lev) {
    return;
  }
  duration = lev->duration;
}
