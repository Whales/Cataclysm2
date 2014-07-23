#include "monster_ability.h"
#include "geometry.h" // For Tripoint
#include <sstream>
#include <map> // To help with messages

Monster_ability::Monster_ability()
{
  frequency = 0;
  AP_cost = 0;
  HP_cost = 0;
}

Monster_ability::~Monster_ability()
{
}

Monster_ability_summon::Monster_ability_summon()
{
  number = Dice(0, 0, 1);
  range = 1;
  max_summons = 0;
}

Monster_ability_signal::Monster_ability_signal()
{
  range = 1;
}

Monster_ability_terrain::Monster_ability_terrain()
{
  always_replace = false;
  range = 1;
  tiles_affected = Dice(0, 0, 1);
}

Monster_ability_teleport::Monster_ability_teleport()
{
  range = 1;
  always_use_max_range = false;
  controlled = false;
  phase = false;
}

Monster_ability_fields::Monster_ability_fields()
{
  range = 1;
  affect_all_tiles = false;
}

bool Monster_ability::load_data(std::istream& data, std::string owner)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      debugmsg("Couldn't read file for loading Monster_ability (%s)",
               owner.c_str());
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment; clear the line.
      std::getline(data, junk);

    } else if (ident == "freq:" || ident == "frequency:") {
      data >> frequency;
      std::getline(data, junk);

    } else if (ident == "ap_cost:") {
      data >> AP_cost;
      std::getline(data, junk);

    } else if (ident == "hp_cost:") {
      data >> HP_cost;
      std::getline(data, junk);

    } else if (ident == "verb:") {
      std::getline(data, verb);
      verb = trim(verb);

    } else if (ident != "done" && !handle_data(ident, data)) {
      debugmsg("Unknown Monster_ability property '%s' (%s)",
               ident.c_str(), owner.c_str());
      return false;
    }
  }
  return true;
}

bool Monster_ability::handle_data(std::string ident, std::istream& data,
                                  std::string owner)
{
  return false;
}

bool Monster_ability::effect(Entity* user)
{
  debugmsg("Monster_ability::effect() called!  This is a NULL ability!");
  return false;
}

bool Monster_ability_summon::handle_data(std::string ident, std::istream& data,
                                         std::string owner)
{
  std::string junk;

  if (ident == "monster:") {
    std::string line;
    std::getline(data, line);
    std::istringstream name_data(line);
    if (!monster.load_data(name_data, owner)) {
      debugmsg("Failed to load monster list for Monster_ability_summon (%s)",
               owner.c_str());
      return false;
    }

  } else if (ident == "number:") {
    if (!number.load_data(data, owner)) {
      debugmsg("Failed to load number for Monster_ability_summon (%s)",
               owner.c_str());
      return false;
    }

  } else if (ident == "range:") {
    data >> range;
    std::getline(data, junk);

  } else if (ident == "max_summons:") {
    data >> max_summons;
    std::getline(data, junk);

  } else {
    debugmsg("Unknown Monster_ability_summon property '%s' (%s)",
             ident.c_str(), owner.c_str());
    return false;
  }
  return true;
}

bool Monster_ability_summon::effect(Monster* user)
{
  if (!user) {
    debugmsg("Monster_ability_summon::effect(NULL) called!");
    return false;
  }

  if (monster.empty()) {
    debugmsg("%s tried to use summon ability, but no summons were listed!",
             user->get_data_name().c_str());
    return false; // Unable to pick a monster!
  }

  if (range <= 0) {
    debugmsg("%s tried to use summon ability, but range was %d!",
             user->get_data_name().c_str(), range);
    return false; // No range!
  }

  if (user->summons_used >= max_summons) {
    return false;
  }

  std::vector<Tripoint> valid_targets;
  Tripoint p;
  for (p.x = user->pos.x - range; p.x <= user->pos.x + range; p.x++) {
    for (p.y = user->pos.y - range; p.y <= user->pos.y + range; p.y++) {
      p.z = user->pos.z;
      if (p == user->pos) { // Skip central tile
        p.y++;
      }
      if (GAME.is_empty(p)) {
        valid_targets.push_back(p);
      }
    }
  }

  int spawns = number.roll();

  std::map<std::string,int> actual_spawns;

  while (spawns > 0 && user->summons_used < max_summons &&
         !valid_targets.empty()) {
    spawns--;
    user->summons_used++;

    std::string mon_name = monster.pick();

    Monster* mon = new Monster(mon_name);
    if (!mon->type) {
      debugmsg("%s tried to summon a '%s' but that doesn't exist.",
               user->get_data_name().c_str(), mon_name.c_str());
      delete mon;
      return false;
    }

    int index = rng(0, valid_targets.size() - 1);
    mon->pos = valid_targets[index];
    valid_targets.erase( valid_targets.begin() + index );
    GAME.entities.add_entity(mon);

// Track spawn counts, for the monster below.
    if (GAME.player->can_sense(mon)) {
      if (actual_spawns.count(mon_name) == 0) {
        actual_spawns[mon_name] = 1;
      } else {
        actual_spawns[mon_name]++;
      }
    }

  }

// Display a message if the player sees us.
  int nspn = actual_spawns.size();
  bool see_user = GAME.player->can_sense(user);
  if (nspn > 0) {
    std::stringstream message;
    if (see_user) {
      message << user->get_name_to_player() << " " << verb << " ";
    }
    bool need_and = (nspn > 1);
    for (std::map<std::string,int>::iterator it = actual_spawns.begin();
         it != actual_spawns.end();
         it++) {
      nspn--;
      if (nspn <= 0 && need_and) {
        message << " and ";
      }
      if (it->second == 1) {
        message << "a " << it->first;
      } else {
        message << it->second << " " << it->first << "s";
      }
      if (nspn == 1) {
        message << " ";
      } else if (nspn >= 2) {
        message << ", ";
      }
    }
    if (!see_user) {
      message << " suddenly appear";
    }
    message << "!";
    GAME.add_msg(message.str());
  }

  return true;
}

bool Monster_ability_signal::handle_data(std::string ident, std::istream& data,
                                         std::string owner)
{
  std::string junk;

  if (ident = "signal:") {
    std::string line;
    std::getline(data, line);
    std::istringstream signal_data(line);
    if (!signal.load_data(signal_data, owner)) {
      debugmsg("Failed to load signal list for Monster_ability_signal (%s)",
               owner.c_str());
      return false;
    }

  } else if (ident == "range:") {
    data >> range;
    std::getline(data, junk);

  } else {
    debugmsg("Unknown Monster_ability_signal property '%s' (%s).",
             ident.c_str(), owner.c_str());
    return false;
  }
  return true;
}

bool Monster_ability_signal::effect(Monster* user)
{
  if (!user) {
    debugmsg("Monster_ability_signal::effect(NULL) called!");
    return false;
  }

  if (signal.empty()) {
    debugmsg("%s tried to use Monster_ability_signal but no signals listed.",
             user->get_name().c_str());
    return false;
  }

  if (range < 0) {
    debugmsg("%s tried to use Monster_ability_signal but range was %d!",
             user->get_name().c_str(), range);
    return false;
  }

  Tripoint pos;
  std::string sig = signal.pick();
  for (pos.x = user->pos.x - range; pos.x <= user->pos.x + range; pos.x++) {
    for (pos.y = user->pos.y - range; pos.y <= user->pos.y + range; pos.y++) {
      pos.z = user->pos.z;
      GAME.map->apply_signal(signal, pos, user);
    }
  }
  return true;
}

bool Monster_ability_terrain::handle_data(std::string ident, std::istream& data,
                                          std::string owner)
{
  std::string junk;

  if (ident == "always_replace") {
    always_replace = true;

  } else if (ident == "damage:") {
    if (!damage.load_data(data, owner)) {
      debugmsg("Monster_ability_terrain failed to load damage (%s)",
               owner.c_str());
      return false;
    }

  } else if (ident == "terrain:") {
    std::string line;
    std::getline(data, line);
    std::istringstream terrain_data(line);
    if (!variable_string.load_data(terrain_data, owner)) {
      debugmsg("Monster_ability_terrain failed to load terrain list (%s).",
               owner.c_str());
      return false;
    }

  } else if (ident == "range:") {
    data >> range;
    std::getline(data, junk);

  } else if (ident == "tiles_affected:") {
    if (!tiles_affected.load_data(data, owner)) {
      debugmsg("Monster_ability_terrain failed to load tiles_affected (%s).",
               owner.c_str());
      return false;
    }

  } else {
    debugmsg("Unknown Monster_ability_terrain property '%s' (%s).",
             ident.c_str(), owner.c_str());
    return false;
  }
  return true;
}

bool Monster_ability_terrain::effect(Monster* user)
{
  if (!user) {
    debugmsg("Monster_ability_terrain::effect(NULL) called!");
    return false;
  }

// Note: an empty terrain list is deliberately allowed here!

  if (range <= 0) {
    debugmsg("%s used Monster_ability_terrain but range was %d!",
             user->get_data_name().c_str(), range);
    return false;
  }

  int tiles = tiles_affected.roll();
// We return true here because it's an *attempt* to use the ability that failed.
  if (tiles < 0) {
    return true;
  }

  std::vector<Tripoint> valid_targets;
  Tripoint pos;
  for (pos.x = user->pos.x - range; pos.x <= user->pos.x + range; pos.x++) {
    for (pos.y = user->pos.y - range; pos.y <= user->pos.y + range; pos.y++) {
      pos.z = user->pos.z;
      if (pos == user->pos) { // Skip terrain the caster is on!
        pos.y++;
      }
      if (!GAME.entities.entity_at(pos)) {
        valid_targets.push_back(pos);
      }
    }
  }

  std::map<std::string,int> effects; // For printing a message.;
  while (tiles > 0 && !valid_targets.empty()) {
    tiles--;

    int index = rng(0, valid_targets.size() - 1);
    Tripoint ter_pos = valid_targets[index];
    valid_targets.erase(valid_targets.begin() + index);

    std::string ter_name;
    Terrain* ter_used = NULL;
    if (!terrain.empty()) {
      ter_name = terrain.pick();
      ter_used = TERRAIN.lookup_name(ter_name);
      if (!ter_used) {
        debugmsg("'%s' is not a terrain! (%s Monster_ability_terrain)",
                 ter_name.c_str(), user->get_data_name().c_str());
        return false;
      }
    }

    Tile* tile_hit = GAME.map->get_tile(ter_pos);
    if (!tile_hit) {
      debugmsg("%s used Monster_ability_terrain and hit a NULL tile at %s!",
               user->get_data_name().c_str(), ter_pos.str().c_str());
      return false;
    }

    if (!always_replace) {
      tile_hit->damage(DAMAGE_BASH, damage.roll());
    }
    if (ter_used && (always_replace || tile_hit->hp <= 0)) {
      tile_hit->set_terrain(ter_used);
// Check if we need to list the terrain
      if (GAME.player->can_see(GAME.map, ter_pos)) {
        if (effects.count(ter_name) == 0) {
          effects[ter_name] = 1;
        } else {
          effects[ter_name]++;
        }
      }
    }
  }

// Done!  Now display a message, if appropriate.
  bool see_user = GAME.player->can_sense(user);
  int nspn = effects.size();
  if (nspn > 0) {
    std::stringstream message;
    if (see_user) {
      message << user->get_name_to_player() << " " << verb << " ";
    }
    bool need_and = (nspn > 1);
    for (std::map<std::string,int>::iterator it = effects.begin();
         it != effects.end();
         it++) {
      nspn--;
      if (nspn <= 0 && need_and) {
        message << " and ";
      }
      if (it->second == 1) {
        message << "a " << it->first;
      } else {
        message << it->second << " " << it->first << "s";
      }
      if (nspn == 1) {
        message << " ";
      } else if (nspn >= 2) {
        message << ", ";
      }
    }
    if (!see_user) {
      message << " suddenly appear";
    }
    message << "!";
    GAME.add_msg(message.str());
  }

  return true;
}

bool Monster_ability_teleport::handle_data(std::string ident,
                                           std::istream& data,
                                           std::string owner)
{
  std::string junk

  if (ident == "range:") {
    data >> range;
    std::getline(data, junk);

  } else if (ident == "always_use_max_range") {
    always_use_max_range = true;

  } else if (ident == "controlled") {
    controlled = true;

  } else if (ident == "phase") {
    phase = true;

  } else {
    debugsmg("Unknown Monster_ability_teleport property '%s' (%s)",
             ident.c_str(), owner.c_str());
    return false;
  }
  return true;
}
    
