#include "field.h"
#include "window.h"     // For debugmsg
#include "stringfunc.h" // For no_caps and trim
#include "terrain.h"    // For Terrain_flag
#include "globals.h"    // For TERRAIN
#include <sstream>

Field_level::Field_level()
{
  duration = 0;
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
  consumption_result = NULL;
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
      std::getline(data, junk);

    } else if (ident == "consumption_result:") {
      std::string terrain_name;
      std::getline(data, terrain_name);
      terrain_name = trim(terrain_name);
      consumption_result = TERRAIN.lookup_name(terrain_name);
      if (consumption_result == NULL) {
        debugmsg("Unknown terrain '%s' (%s)", terrain_name.c_str(),
                 name.c_str());
        return false;
      }

    } else if (ident == "terrain_modifier:") {
      std::string flag_name;
      int mod = 0;
      bool consume = false;
      data >> flag_name >> mod;
      std::getline(data, junk);
      junk = no_caps(junk);
      junk = trim(junk);
      if (junk == "consume") {
        consume = true;
      }
      Terrain_flag tf = lookup_terrain_flag(flag_name);
      if (tf == TF_NULL) {
        debugmsg("Unknown terrain flag '%s' (%s)", flag_name.c_str(),
                 name.c_str());
        return false;
      }
      terrain_modifiers.push_back( Field_terrain_modifier(tf, mod, consume) );

    } else if (ident == "item_modifier:") {
      std::string flag_name;
      int mod = 0;
      bool consume = false;
      data >> flag_name >> mod;
      std::getline(data, junk);
      junk = no_caps(junk);
      junk = trim(junk);
      if (junk == "consume") {
        consume = true;
      }
      Item_flag itf = lookup_item_flag(flag_name);
      if (itf == ITEM_FLAG_NULL) {
        debugmsg("Unknown item flag '%s' (%s)", flag_name.c_str(),
                 name.c_str());
        return false;
      }
      item_modifiers.push_back( Field_item_modifier(itf, mod, consume) );

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
