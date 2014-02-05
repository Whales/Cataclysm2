#include "field.h"
#include "window.h"     // For debugmsg
#include "stringfunc.h" // For no_caps and trim
#include "terrain.h"    // For Terrain_flag
#include "globals.h"    // For TERRAIN and FIELDS
#include "rng.h"
#include "map.h"
#include "entity.h"
#include <sstream>

bool Field_fuel::load_data(std::istream& data, std::string owner_name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      debugmsg("Couldn't read ident for fuel (%s)", owner_name.c_str());
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
      terrain_flag = tf;

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
      item_flag = itf;

    } else if (ident == "fuel:") {
      data >> fuel;
      std::getline(data, junk);

    } else if (ident == "damage:") {
      damage.load_data(data, owner_name + " fuel");

    } else if (ident == "output_field:") {
      std::getline(data, output_field);
      output_field = trim(output_field);

    } else if (ident == "output_duration:") {
      output_duration.load_data(data, owner_name);

    } else if (ident != "done") {
      debugmsg("Unknown Field_terrain_fuel property '%s' (%s)", ident.c_str(),
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
  for (int i = 0; i < FIELD_FLAG_MAX; i++) {
    field_flags.push_back(false);
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

    } else if (ident == "danger:") {
      data >> danger;
      std::getline(data, junk);

    } else if (ident == "verb:") {
      std::getline(data, verb);
      verb = trim(verb);

    } else if (ident == "parts_hit:") {
      std::string body_part_line;
      std::getline(data, body_part_line);
      std::istringstream body_part_data(body_part_line);
      std::string body_part_name;
// TODO: Standardize this somewhere, it's used in Attack too I think?
      while (body_part_data >> body_part_name) {
        if (body_part_name == "arms") {
          body_parts_hit.push_back(BODYPART_LEFT_ARM);
          body_parts_hit.push_back(BODYPART_RIGHT_ARM);
        } else if (body_part_name == "legs") {
          body_parts_hit.push_back(BODYPART_LEFT_LEG);
          body_parts_hit.push_back(BODYPART_RIGHT_LEG);
        } else if (body_part_name == "all") {
          body_parts_hit.clear();
          for (int i = 1; i < BODYPART_MAX; i++) {
            body_parts_hit.push_back( Body_part(i) );
          }
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
/* TODO:  Should terrain flags and field flags be loaded with different property
 *        tags?  Normally it's not an issue, but if there's a name collision we
 *        obviously need to be able to distinguish between the two.
 */
      std::string flag_line;
      std::getline(data, flag_line);
      std::istringstream flag_data(flag_line);
      std::string flag_name;
      while (flag_data >> flag_name) {
        Terrain_flag tf = lookup_terrain_flag(flag_name);
        if (tf == TF_NULL) {
          Field_flag ff = lookup_field_flag(flag_name);
          if (ff == FIELD_FLAG_NULL) {
            debugmsg("'%s' is not a terrain nor a field flag (%s:%s)",
                     flag_name.c_str(), owner_name.c_str(), name.c_str());
            return false;
          } else {
            field_flags[ff] = true;
          }
        } else {
          terrain_flags[tf] = true;
        }
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

bool Field_level::has_flag(Field_flag ff)
{
  return field_flags[ff];
}

bool Field_level::has_flag(Terrain_flag tf)
{
  return terrain_flags[tf];
}

Field_type::Field_type()
{
  uid = -1;
  spread_chance = 0;
  for (int i = 0; i < TF_MAX; i++) {
    terrain_flags.push_back(false);
  }
  for (int i = 0; i < FIELD_FLAG_MAX; i++) {
    field_flags.push_back(false);
  }
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

bool Field_type::has_flag(Terrain_flag tf, int level)
{
  if (terrain_flags[tf]) {
    return true;
  }
  if (level >= 0 && level < levels.size() && get_level(level)->has_flag(tf)) {
    return true;
  }
  return false;
}

bool Field_type::has_flag(Field_flag ff, int level)
{
  if (terrain_flags[ff]) {
    return true;
  }
  if (level >= 0 && level < levels.size() && get_level(level)->has_flag(ff)) {
    return true;
  }
  return false;
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

    } else if (ident == "flags:") {
/* TODO:  Should terrain flags and field flags be loaded with different property
 *        tags?  Normally it's not an issue, but if there's a name collision we
 *        obviously need to be able to distinguish between the two.
 */
      std::string flag_line;
      std::getline(data, flag_line);
      std::istringstream flag_data(flag_line);
      std::string flag_name;
      while (flag_data >> flag_name) {
        Terrain_flag tf = lookup_terrain_flag(flag_name);
        if (tf == TF_NULL) {
          Field_flag ff = lookup_field_flag(flag_name);
          if (ff == FIELD_FLAG_NULL) {
            debugmsg("'%s' is not a terrain nor a field flag (%s)",
                     flag_name.c_str(), name.c_str());
            return false;
          } else {
            field_flags[ff] = true;
          }
        } else {
          terrain_flags[tf] = true;
        }
      }


    } else if (ident == "spread_chance:") {
      data >> spread_chance;
      if (spread_chance < 0 || spread_chance > 100) {
        debugmsg("Bad spread_chance %d (%s)", spread_chance, name.c_str());
        return false;
      }
      std::getline(data, junk);

    } else if (ident == "spread_cost:") {
      data >> spread_cost;
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

    } else if (ident == "fuel:") {
      Field_fuel tmpfuel;
      if (tmpfuel.load_data(data, name)) {
        fuel.push_back(tmpfuel);
      } else {
        debugmsg("Fuel failed to load (%s)", name.c_str());
        return false;
      }

    } else if (ident == "level:") {
      Field_level* tmp_level = new Field_level;
      if (!tmp_level->load_data(data, name)) {
        delete tmp_level;
        debugmsg("Field level failed to load (%s)", name.c_str());
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

int Field::get_type_uid() const
{
  if (!type) {
    return -1;
  }
  return type->uid;
}

bool Field::is_valid()
{
  if (!type) {
    return false;
  }
  if (level < 0) {
    return false;
  }
  if (dead) {
    return false;
  }
  return true;
}

bool Field::has_flag(Field_flag flag)
{
  if (!type) {
    return false;
  }
  Field_level* lev = type->get_level(level);
  if (!lev) {
    return false;
  }
  return lev->has_flag(flag);
}

bool Field::has_flag(Terrain_flag flag)
{
  if (!type) {
    return false;
  }
  Field_level* lev = type->get_level(level);
  if (!lev) {
    return false;
  }
  return lev->has_flag(flag);
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

glyph Field::top_glyph()
{
  if (!type) {
    return glyph();
  }
  Field_level* lev = type->get_level(level);
  return lev->sym;
}

int Field::get_full_duration() const
{
  int ret = duration;
  if (!type) {
    return ret;
  }
  for (int i = 0; i < level; i++) {
    ret += type->get_level(i)->duration;
  }
  return ret;
}

Field& Field::operator+=(const Field& rhs)
{
  if (get_type_uid() != rhs.get_type_uid()) {
    return (*this);
  }
  duration += rhs.get_full_duration();
  adjust_level();
  return (*this);
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
void Field::process(Map* map, Tripoint pos)
{
  if (!map) {
    debugmsg("Field::process() called with map == NULL!");
    return;
  }
  if (!type) {
    debugmsg("Field::process() called for a Field without any type!");
    return;
  }

// Fetch info on the map and our type data
  Field_level* level_data = type->get_level(level);

  duration--;

  bool found_fuel = consume_fuel(map, pos);
// Lose extra duration if we didn't find fuel
  if (!found_fuel && level_data) {
    duration -= level_data->duration_lost_without_fuel;
  }

// Spread if appropriate
// TODO: Vertical spreading?

// Calculate which points are open for spreading
  if ((has_flag(FIELD_FLAG_DIFFUSE) || duration > type->spread_cost) &&
      rng(1, 100) <= type->spread_chance) {
    bool solid = has_flag(FIELD_FLAG_SOLID);
    std::vector<Tripoint> spread_points;
    for (int x = pos.x - 1; x <= pos.x + 1; x++) {
      for (int y = pos.y - 1; x <= pos.y + 1; y++) {
        int z = pos.z;
        if ((!map->contains_field(x, y, z) ||
            map->field_uid_at(x, y, z) == get_type_uid()) &&
            (solid || map->move_cost(x, y, z) > 0))  {
          spread_points.push_back( Tripoint(x, y, z) );
        }
      }
    }

    if (!spread_points.empty()) {
      Tripoint spread = spread_points[ rng(0, spread_points.size() - 1) ];
      map->add_field(type, spread, creator);
      duration -= type->spread_cost;
    }
  }

// Change our level, if appropriate!
  adjust_level();

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

void Field::adjust_level()
{
  if (!type) {
    return;
  }

  debugmsg("Duration %d; level => %d", duration, level);
  while (duration < 0 && level >= 0) {
    level--;
    duration += type->get_level(level)->duration;
    debugmsg("Duration %d; level => %d", duration, level);
  }

  while (duration >= type->duration_needed_to_reach_level(level + 1)) {
    duration -= type->get_level(level)->duration;
    level++;
    debugmsg("Duration %d; level => %d", duration, level);
  }

  if (duration < 0) {
    dead = true;
  }
}

bool Field::consume_fuel(Map* map, Tripoint pos)
{
// Calculate which points are open - in case we spread or put out smoke/etc
  std::vector<Tripoint> open_points_all;
  std::vector<Tripoint> open_points_passable; // i.e. not walls
  for (int x = pos.x - 1; x <= pos.x + 1; x++) {
    for (int y = pos.y - 1; x <= pos.y + 1; y++) {
      int z = pos.z;
      if (!map->contains_field(x, y, z)) {
        open_points_all.push_back( Tripoint(x, y, z) );
        if (map->move_cost(x, y, z) > 0) {
          open_points_passable.push_back( Tripoint(x, y, z) );
        }
      }
    }
  }

// Get info on the tile and our field data
  Tile* tile_here = map->get_tile(pos);

  bool found_fuel = false;  // Our return value
  std::vector<Field>  output; // Fields we output (e.g. smoke)
// Check for fuel/extinguishers
  for (std::list<Field_fuel>::iterator it = type->fuel.begin();
       it != type->fuel.end();
       it++) {
    Field_fuel fuel = (*it);
    int damage = fuel.damage.roll();
// Create a new field that we're going to output
    Field_type* smoke_type = FIELDS.lookup_name(fuel.output_field);
    if (!fuel.output_field.empty() && !smoke_type) {
      debugmsg("Couldn't find field type '%s' (Field::consume_fuel() for '%s'",
               fuel.output_field.c_str(), get_name().c_str());
      return false;
    }
    Field tmp_field(smoke_type, 1, creator);
    tmp_field.duration = 0;
// Check for terrain flag
    if (tile_here->has_flag(fuel.terrain_flag)) {
      found_fuel = true;
      int fuel_gained = fuel.fuel;
      if (tile_here->hp < damage) {
        double fuel_percent = tile_here->hp / damage;
        fuel_gained = int( double(fuel.fuel) * fuel_percent );
      }
      duration += fuel_gained;
      tmp_field.duration += fuel.output_duration.roll();
// TODO: Assign a damage type to Field_terrain_fuel?
      tile_here->damage(DAMAGE_NULL, damage);
    }
// Check items at this tile
    for (int n = 0; n < tile_here->items.size(); n++) {
      Item* it = &(tile_here->items[n]);
      if (it->has_flag(fuel.item_flag)) {
        found_fuel = true;
        int fuel_gained = fuel.fuel;
        if (it->hp < damage) {
          double fuel_percent = it->hp / damage;
          fuel_gained = int( double(fuel.fuel) * fuel_percent );
        }
        duration += fuel_gained;
        tmp_field.duration += fuel.output_duration.roll();
        if (it->damage(damage)) { // Item was destroyed
          tile_here->items.erase( tile_here->items.begin() + n );
          n--;
        }
      }
    }
// Push our output to the vector of new fields
// Ensure tmp_field.duration > 0 because sometimes it rolls < 0
    if (tmp_field.type && tmp_field.duration > 0) {
      tmp_field.adjust_level();
      output.push_back(tmp_field);
    }
  }

// At this point we've consumed fuel; so output whatever smoke we have
  for (int i = 0; i < output.size(); i++) {
    std::vector<Tripoint>* placement;
// Decide whether to place on all valid points, or passable ones
    if ( output[i].has_flag(FIELD_FLAG_SOLID) ) {
      placement = &(open_points_all);
    } else {
      placement = &(open_points_passable);
    }
    int index = rng(0, placement->size() - 1);
    Tripoint p = (*placement)[index];
    placement->erase( placement->begin() + index );
    map->add_field(output[i], p);
  }

  return found_fuel;
}


Field_flag lookup_field_flag(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < FIELD_FLAG_MAX; i++) {
    Field_flag ret = Field_flag(i);
    if ( no_caps( field_flag_name(ret) ) == name ) {
      return ret;
    }
  }
  return FIELD_FLAG_NULL;
}

std::string field_flag_name(Field_flag flag)
{
  switch (flag) {
    case FIELD_FLAG_NULL:     return "NULL";
    case FIELD_FLAG_SOLID:    return "solid";
    case FIELD_FLAG_DIFFUSE:  return "diffuse";
    case FIELD_FLAG_MAX:      return "BUG - FIELD_FLAG_MAX";
    default:                  return "BUG - Unnamed Field_flag";
  }
  return "BUG - Escaped field_flag_name switch";
}
