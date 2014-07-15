#include "field.h"
#include "map.h"
#include "rng.h"
#include "globals.h"
#include "monster.h"
#include "game.h"
#include "attack.h"
#include "entity.h"
#include "enum.h"
#include "worldmap.h"
#include "files.h"    // For SAVE_DIR
#include <fstream>
#include <sstream>

Furniture::Furniture()
{
  type = NULL;
  uid  = -1;
}

Furniture::~Furniture()
{
}

void Furniture::set_type(Furniture_type* t)
{
  type = t;
  if (type) {
    hp = type->hp;
  }
}

void Furniture::set_uid(int id)
{
  uid = id;
}

bool Furniture::is_real()
{
  return (type);
}

int Furniture::get_uid()
{
  return uid;
}

glyph Furniture::get_glyph()
{
  if (!type) {
    return glyph();
  }
  glyph ret = type->sym;
  if (is_smashable() && hp > 0 && hp < type->hp) {
    int percent = (100 * hp) / type->hp;
    if (percent >= 80) {
      ret = ret.hilite(c_green);
    } else if (percent >= 40) {
      ret = ret.hilite(c_brown);
    } else {
      ret = ret.hilite(c_red);
    }
  }
  return ret;
}

int Furniture::move_cost()
{
  if (!type) {
    return 100;
  }
  return type->move_cost;
}

int Furniture::get_height()
{
  if (!type) {
    return 0;
  }
  return type->height;
}

int Furniture::get_weight()
{
  if (!type) {
    return 0;
  }
  return type->weight;
}

std::string Furniture::get_name()
{
  if (!type) {
    return "";
  }
  return type->get_name();
}

bool Furniture::has_flag(Terrain_flag flag)
{
  return (type && type->has_flag(flag));
}

bool Furniture::is_smashable()
{
  return (type && type->smashable);
}

std::string Furniture::smash(Damage_set dam)
{
  if (!is_smashable()) {  // This verifies that terrain != NULL
    return "";
  }
  Terrain_smash smash = type->smash;
  if (rng(1, 100) <= smash.ignore_chance) {
    return smash.failure_sound; // Make our "saving throw"
  }
  if (damage(dam)) {
    return smash.success_sound;
  }
  return smash.failure_sound;
}

// Roll all damage types, but only apply whichever is the best.
bool Furniture::damage(Damage_set dam)
{
  if (!type || type->hp == 0) {
    return false;
  }

  int best_dmg = 0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    Damage_type damtype = Damage_type(i);
    int dmg = dam.get_damage(damtype) - type->smash.armor[damtype].roll();
    if (dmg > best_dmg) {
      best_dmg = dmg;
    }
  }

  hp -= best_dmg;
  if (hp <= 0) {
    return true;
  }
  return false;
}

bool Furniture::damage(Damage_type damtype, int dam)
{
  if (dam <= 0) {
    return false;
  }
  if (!type || type->hp == 0) {
    return false;
  }
  Dice armor = type->smash.armor[damtype];
  dam -= armor.roll();
  if (dam <= 0) {
    return false;
  }
  hp -= dam;
  if (hp <= 0) {
    return true;
  }
  return false;
}

void Furniture::destroy()
{
  type = NULL;
  uid  = -1;
}

std::string Furniture::save_data()
{
  if (!type) {
    return "Done";
  }

  std::stringstream ret;

  ret << "Type: " << type->name << std::endl;  // Name is a persistant unique ID
  ret << "HP: " << hp << std::endl;
  ret << "UID: " << uid << std::endl;
  ret << "Done";

  return ret.str();
}

bool Furniture::load_data(std::istream& data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      debugmsg("Couldn't read Furniture data.");
      return false;
    }
    ident = no_caps( ident );

    if (ident == "type:") {
      std::string tmpname;
      std::getline(data, tmpname);
      tmpname = trim( tmpname );
      type = FURNITURE_TYPES.lookup_name(tmpname);
      if (!type) {
        debugmsg("Unknown furniture '%s'", tmpname.c_str());
        return false;
      }

    } else if (ident == "hp:") {
      data >> hp;
      std::getline(data, junk);

    } else if (ident == "uid:") {
      data >> uid;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown furniture identifier '%s'", ident.c_str());
      return false;
    }
  }
  return true;
}

void Tile::set_terrain(Terrain* ter)
{
  if (!ter) {
    debugmsg("Tile::set_terrain(NULL)!");
    return;
  }
  terrain = ter;
  hp = ter->hp;
}

void Tile::add_furniture(Furniture_type* type, int uid)
{
  if (!type) {
    debugmsg("Tile::add_furniture(NULL)!");
    return;
  }

  furniture.set_type(type);
  furniture.set_uid(uid);
}

void Tile::add_furniture(Furniture furn)
{
  furniture = furn;
}

void Tile::remove_furniture()
{
  furniture.set_type(NULL);
}

glyph Tile::top_glyph()
{
  if (field.is_valid()) {
    return field.top_glyph();
  }
  if (furniture.is_real()) {
    return furniture.get_glyph();
  }
  if (!items.empty() && (!has_flag(TF_SEALED) || !has_flag(TF_OPAQUE))) {
    if (terrain && !terrain->has_flag(TF_FLOOR)) {
      return terrain->sym.hilite(c_blue);
    }
    glyph ret = items.back().top_glyph();
    if (items.size() > 1) {
      ret = ret.invert();
    }
    return ret;
  }
  if (!terrain) {
    return glyph();
  }
  glyph ret = terrain->sym;
  if (is_smashable() && terrain->hp > 0 && hp < terrain->hp) {
    int percent = (100 * hp) / terrain->hp;
    if (percent >= 80) {
      ret = ret.hilite(c_green);
    } else if (percent >= 40) {
      ret = ret.hilite(c_brown);
    } else {
      ret = ret.hilite(c_red);
    }
  }
  return ret;
}

int Tile::move_cost()
{
  if (furniture.is_real()) {
    return furniture.move_cost();
  }
  if (!terrain) {
    return 0;
  }
  return (terrain->movecost);
}

int Tile::get_height()
{
  int ret = (terrain ? terrain->height : 0);
  if (furniture.is_real()) {
    ret += furniture.get_height();
  }
  return ret;
}

std::string Tile::get_name()
{
  std::stringstream ret;
  if (furniture.is_real()) {
    ret << furniture.get_name() << " on ";
  }
  ret << (terrain ? terrain->get_name() : "<c=red>BUG - Unknown<c=/>");

  return ret.str();
}

std::string Tile::get_name_indefinite()
{
  std::stringstream ret;
  if (furniture.is_real()) {
    ret << (furniture.has_flag(TF_PLURAL) ? "some" : "a") << " " <<
           furniture.get_name() << " on ";
  }
  if (terrain) {
    ret << (terrain->has_flag(TF_PLURAL) ? "some" : "a") << " " <<
           terrain->get_name();
  } else { 
    ret << "<c=red>BUG - Unknown<c=/>";
  }
  return ret.str();
}

bool Tile::blocks_sense(Sense_type sense, int z_value)
{
  if (!terrain) {
    return false;
  }

  switch (sense) {

    case SENSE_NULL:
      return true;

    case SENSE_SIGHT:
      if (field.is_valid() && field.has_flag(TF_OPAQUE)) {
        return true;
      } else if (has_flag(TF_OPAQUE) && z_value <= get_height()) {
        return true;
      }
      return false;

    case SENSE_SOUND:
      return false;

    case SENSE_ECHOLOCATION:
      return (move_cost() == 0);

    case SENSE_SMELL:
      return (move_cost() == 0);

    case SENSE_OMNISCIENT:
      return false;

    case SENSE_MAX:
      return false;

  }
  return false;
}

bool Tile::has_flag(Terrain_flag flag)
{
  if (field.is_valid() && field.has_flag(flag)) {
    return true;
  }
  if (!terrain) {
    return false;
  }
  return terrain->has_flag(flag);
}

bool Tile::has_field()
{
  return field.is_valid();
}

bool Tile::has_furniture()
{
  return furniture.is_real();
}

bool Tile::is_smashable()
{
  if (furniture.is_real() && furniture.is_smashable()) {
    return true;
  }
  return (terrain && terrain->can_smash());
}

std::string Tile::smash(Damage_set dam)
{
// First check furniture
  if (furniture.is_real()) {
    std::string sound = furniture.smash(dam);
    if (furniture.hp <= 0) { // We destroyed the furniture!
// First, add all items in the furniture's type list
      Item_group* furn_items = furniture.type->components;
      if (furn_items) {
        for (int i = 0; i < furn_items->item_types.size(); i++) {
          Item it(furn_items->item_types[i].item);
          for (int n = 0; n < furn_items->item_types[i].number; n++) {
            items.push_back(it);
          }
        }
      }
// Next, destroy the furniture
      furniture.destroy();
    }
    return sound; // We smashed furniture, we don't get to smash terrain too!
  }
      
  if (!is_smashable()) {  // This also verifies that terrain != NULL
    return "";
  }

  Terrain_smash smash = terrain->smash;

  if (rng(1, 100) <= smash.ignore_chance) {
    return smash.failure_sound; // Make our "saving throw"
  }

  if (damage(dam)) {
    return smash.success_sound;
  }

  return smash.failure_sound;
}

// Roll all damage types; but only actually use the very best one.
bool Tile::damage(Damage_set dam)
{
  if (!terrain || terrain->hp == 0) {
    return false;
  }

  int best_dmg = 0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    Damage_type damtype = Damage_type(i);
    int dmg = dam.get_damage(damtype) - terrain->smash.armor[damtype].roll();
    if (dmg > best_dmg) {
      best_dmg = dmg;
    }
  }

  hp -= best_dmg;
  if (hp <= 0) {
    destroy();
    return true;
  }
  return false;
}

bool Tile::damage(Damage_type type, int dam)
{
  if (dam <= 0) {
    return false;
  }
  if (!terrain || terrain->hp == 0) {
    return false;
  }
  Dice armor = terrain->smash.armor[type];
  dam -= armor.roll();
  if (dam <= 0) {
    return false;
  }
  hp -= dam;
  if (hp <= 0) {
    destroy();
    return true;
  }
  return false;
}

void Tile::destroy()
{
// If HP is negative, then we run damage *again* with the extra damage
  int extra = 0 - hp;
  Terrain* result = TERRAIN.lookup_name( terrain->destroy_result );
  if (!result) {
    debugmsg("Tried to destroy '%s' but couldn't look up result '%s'.",
             get_name().c_str(), terrain->destroy_result.c_str());
  } else {
    set_terrain(result);
    damage(DAMAGE_NULL, extra);  // See above
  }
}

bool Tile::signal_applies(std::string signal)
{
  signal = no_caps(signal);
  signal = trim(signal);
  if (!terrain || terrain->signal_handlers.count(signal) == 0) {
    return false;
  }
  return true;
}

bool Tile::apply_signal(std::string signal, Entity* user)
{
  signal = no_caps(signal);
  signal = trim(signal);
  std::string user_name = "";

  if (user) {
    user_name = user->get_name_to_player();
  }

  if (!terrain || !signal_applies(signal)) {
    if (user) {
      GAME.add_msg("Nothing to %s there.", user_name.c_str(), signal.c_str());
    }
    return false;
  }

  Terrain_signal_handler handler = terrain->signal_handlers[signal];

  int success = handler.success_rate;
// Apply bonuses, if the user exists
  if (user) {
// Terrain bonuses - check the flags for the terrain the user is on
// Kind of weird to check GAME.map from a tile, but... eh
    Tile* user_tile = GAME.map->get_tile(user->pos);
    if (user_tile) {
      for (std::list<Terrain_flag_bonus>::iterator it =
              handler.terrain_flag_bonuses.begin();
           it != handler.terrain_flag_bonuses.end();
           it++) {
        if (user_tile->has_flag( it->flag )) {
          success += it->amount;
        }
      }
    }
// Stat bonuses
    for (std::list<Stat_bonus>::iterator it = handler.stat_bonuses.begin();
         it != handler.stat_bonuses.end();
         it++) {
      int stat = 0;
      switch (it->stat) {
        case STAT_STRENGTH:     stat = user->stats.strength;      break;
        case STAT_DEXTERITY:    stat = user->stats.dexterity;     break;
        case STAT_INTELLIGENCE: stat = user->stats.intelligence;  break;
        case STAT_PERCEPTION:   stat = user->stats.perception;    break;
      }
// Apply stat in different ways, depending on the operator used...
      switch (it->op) {

        case MATH_MULTIPLY:
          success += stat * it->amount;
          break;

        case MATH_GREATER_THAN:
          if (stat > it->amount) {
            success += it->amount_static;
          }
          break;

        case MATH_GREATER_THAN_OR_EQUAL_TO:
          if (stat >= it->amount) {
            success += it->amount_static;
          }
          break;

        case MATH_LESS_THAN:
          if (stat < it->amount) {
            success += it->amount_static;
          }
          break;

        case MATH_LESS_THAN_OR_EQUAL_TO:
          if (stat <= it->amount) {
            success += it->amount_static;
          }
          break;

        case MATH_EQUAL_TO:
          if (stat == it->amount) {
            success += it->amount_static;
          }
          break;

        default:
          debugmsg("Tile::apply_signal encountered unknown operator");
          return false;
      } // switch (it->symbol)
    } // Iterator over handler.bonuses
  } // if (user)

// We've finalized our success rate; now roll against it

  if (success <= 0) {
    if (user) {
      GAME.add_msg("<c=red>%s (0 percent success rate)<c=/>",
                   handler.failure_message.c_str());
    }
    return true;  // True since we *tried*

  } else if (rng(1, 100) <= success) {
// Success!
    if (handler.success_message.empty()) {
      if (user) {
        GAME.add_msg("<c=ltred>%s %s the %s.<c=/>",
                     user_name.c_str(), signal.c_str(), get_name().c_str());
      }
    } else if (user) {
      std::stringstream mes;
      mes << "<c=ltred>" << handler.success_message << "<c=/>";
      GAME.add_msg(mes.str());
    }
    Terrain* result = TERRAIN.lookup_name(handler.result);
    if (!result) {
      debugmsg("Tile::apply_signal couldn't find terrain '%s'! (%s)",
               handler.result.c_str(), get_name().c_str());
      return false;
    }
    terrain = result;
    return true;
  }
// Failure.
  if (user) {
    GAME.add_msg( handler.failure_message );
  }
  return true;  // True cause we still *tried* to...
}

std::string Tile::save_data()
{
  if (!terrain) {
    return "Done";
  }

  std::stringstream ret;

  ret << "Type: " << terrain->name << std::endl; // Name is a persistant UID
  ret << "HP: " << hp << std::endl;
  if (field.is_valid()) {
    ret << "Field: " << field.save_data() << std::endl;
  }
  if (furniture.is_real()) {
    ret << "Furniture: " << std::endl << furniture.save_data() << std::endl;
  }
  for (int i = 0; i < items.size(); i++) {
    ret << "Item: " << std::endl << items[i].save_data() << std::endl;
  }

  ret << "Done";

  return ret.str();
}

bool Tile::load_data(std::istream& data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      debugmsg("Couldn't read data (Tile).");
      return false;
    }
    ident = no_caps(ident);

    if (ident == "type:") {
      std::string tmpname;
      std::getline(data, tmpname);
      tmpname = trim(tmpname);
      terrain = TERRAIN.lookup_name(tmpname);
      if (!terrain) {
        debugmsg("Unknown Terrain '%s'", tmpname.c_str());
        return false;
      }

    } else if (ident == "hp:") {
      data >> hp;
      std::getline(data, junk);

    } else if (ident == "field:") {
      if (!field.load_data(data)) {
        field = Field();
      }

    } else if (ident == "furniture:") {
      if (!furniture.load_data(data)) {
        furniture = Furniture();
      }

    } else if (ident == "item:") {
      Item tmpit;
      if (tmpit.load_data(data)) {
        items.push_back(tmpit);
      }

    } else if (ident != "done") {
      debugmsg("Unknown Tile property '%s'", ident.c_str());
      return false;
    }
  }
  return true;
}

Submap::Submap()
{
  spec_used = NULL;
  rotation = DIR_NULL;
  level = 0;
}

Submap::~Submap()
{
}

void Submap::generate_empty()
{
  Terrain* grass = TERRAIN.lookup_name("grass");
  Terrain* dirt  = TERRAIN.lookup_name("dirt");
  if (!grass || !dirt) {
    debugmsg("Couldn't find terrain for generate_empty()");
    return;
  }

  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      tiles[x][y].set_terrain(one_in(2) ? grass : dirt);
    }
  }
}

void Submap::generate_open()
{
  Terrain* open = TERRAIN.lookup_name("empty");
  if (!open) {
    debugmsg("Couldn't find terrain 'empty'; Submap::generate_open()");
    return;
  }

  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      tiles[x][y].set_terrain(open);
    }
  }
}

void Submap::generate(Worldmap* map, int posx, int posy, int posz)
{
  if (!map) {
    debugmsg("Submap::generate(NULL, %d, %d)", posx, posy);
    generate_empty();
    return;
  }
  Worldmap_tile *tile = map->get_tile(posx, posy);
  if (!tile) {
    generate_empty();
    return;
  }
  World_terrain* ter[5];
  ter[0] = tile->terrain;
// North
  tile = map->get_tile(posx, posy - 1);
  ter[1] = (tile ? tile->terrain : NULL);
// East
  tile = map->get_tile(posx + 1, posy);
  ter[2] = (tile ? tile->terrain : NULL);
// South
  tile = map->get_tile(posx, posy + 1);
  ter[3] = (tile ? tile->terrain : NULL);
// West
  tile = map->get_tile(posx - 1, posy);
  ter[4] = (tile ? tile->terrain : NULL);

  generate(ter, posz);
}

void Submap::generate(World_terrain* terrain[5], int posz)
{
  if (!terrain[0]) {
    generate_empty();
  } else {
    std::vector<bool> neighbor;
    Mapgen_spec* spec = NULL;
// We shouldn't ever hit this; Mapgen_pool handles above-ground.  But safety!
    if (posz > 0) {
      generate_open();
    } else if (terrain[0]->has_flag(WTF_RELATIONAL)) {
      neighbor.push_back(false);
      for (int i = 1; i < 5; i++) {
        bool nb = (terrain[i] == terrain[0]);
        for (int n = 0; !nb && n < terrain[0]->connectors.size(); n++) {
          std::string conn = no_caps( terrain[0]->connectors[n] );
          if ( no_caps( terrain[i]->get_data_name() ) == conn ) {
            nb = true;
          }
        }
        neighbor.push_back(nb);
      }
      spec = MAPGEN_SPECS.random_for_terrain(terrain[0], neighbor);
    } else {
      spec = MAPGEN_SPECS.random_for_terrain(terrain[0], "", 0);
    }
    if (!spec) {
      int num_conn = 0;
      for (int i = 0; i < neighbor.size(); i++) {
        if (neighbor[i]) {
          num_conn++;
        }
      }
      debugmsg("Mapgen::generate() failed to find spec for %s [conn=%d, z=%d]",
               terrain[0]->get_data_name().c_str(), num_conn, posz);
      generate_empty();
      return;
    }
    spec->prepare(terrain);
    generate( spec );
  }

// If we're above ground, DON'T do adjacency maps!
  if (posz > 0) {
    return;
  }

// Now do adjacency maps
  for (int i = 1; i < 5; i++) {
    if (terrain[i] && terrain[i] != terrain[0]) {
      Mapgen_spec* adj = MAPGEN_SPECS.random_adjacent_to(terrain[i],terrain[0]);
      if (adj) {
        adj->prepare(terrain);
        adj->rotate( Direction(i) );
        generate_adjacent( adj );
      }
    }
  }
}

void Submap::generate(Mapgen_spec* spec)
{
  if (!spec) {
    debugmsg("Null spec in Submap::generate()!");
    generate_empty();
    return;
  }
// Set our subname to the spec's subname (defaults to empty, only matters for
// multi-story buildings
// Ditto rotation.
  spec_used = spec;
  subname = spec->subname;
// Rotation gets set in Mapgen_spec::prepare(), so it should still be valid here
  rotation = spec->rotation;
// First, set the terrain.
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      Terrain* ter = spec->pick_terrain(x, y);
      if (!ter) {
        debugmsg("Generating null terrain at [%d:%d] (%s)", x, y,
                 spec->get_name().c_str());
        spec->debug_output();
      }
      tiles[x][y].set_terrain(ter);
    }
  }

// Next, add any furniture that needs adding.
// The Game keeps track of furniture UIDs, but so do Mapgen_specs.
// So store a std::map of what each Mapgen UID should be translated to.
  std::map<int,int> map_uid_to_game_uid;
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      Furniture_type* furniture = spec->pick_furniture(x, y);
      if (furniture) {
        int map_uid = spec->pick_furniture_uid(x, y);
        if (map_uid_to_game_uid.count(map_uid) == 0) {
          map_uid_to_game_uid[map_uid] = GAME.get_furniture_uid();
        }
        tiles[x][y].add_furniture(furniture, map_uid_to_game_uid[map_uid]);
      }
    }
  }
  
// Next, add items.
  for (std::map<char,Item_area>::iterator it = spec->item_defs.begin();
       it != spec->item_defs.end();
       it++) {
    Item_area* area = &(it->second);
    area->reset();
    while (area && area->place_item()) {
      Point p = area->pick_location();
      Item item( area->pick_type(spec->get_name()) );
      if (item.type) {
        item.prep_for_generation();
        add_item(item, p.x, p.y);
      }
    }
  }

// Item_group_amount_areas work similarly!
  for (std::map<char,Item_group_amount_area>::iterator
        it = spec->item_group_defs.begin();
       it != spec->item_group_defs.end();
       it++) {
    Item_group_amount_area* area = &(it->second);
    Item_group_amount group = area->pick_group();
    int amount = group.amount.roll();
    for (int i = 0; i < amount; i++) {
      Point p = area->pick_location();
      Item item( group.group->pick_type() );
      item.prep_for_generation();
      add_item(item, p.x, p.y);
    }
  }

// Item_amount_areas work the same as Item_group_amount_areas, more or less
  for (std::map<char,Item_amount_area>::iterator
        it = spec->item_amount_defs.begin();
       it != spec->item_amount_defs.end();
       it++) {
    Item_amount_area* area = &(it->second);
    Item_amount item_amt = area->pick_item();
    int amount = item_amt.amount.roll();
    for (int i = 0; i < amount; i++) {
      Point p = area->pick_location();
      Item item( item_amt.item );
      item.prep_for_generation();
      add_item(item, p.x, p.y);
    }
  }
}

void Submap::generate_adjacent(Mapgen_spec* spec)
{
  if (spec == NULL) {
    return;
  }
// First, set the terrain.
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      Terrain* tmpter = spec->pick_terrain(x, y);
// TODO: Only overwrite terrain with the "ground" tag
      if (tmpter &&
          (!tiles[x][y].terrain || tiles[x][y].terrain->has_flag(TF_MUTABLE))) {
        tiles[x][y].set_terrain(tmpter);
      }
    }
  }
// Next, add items.
  for (std::map<char,Item_area>::iterator it = spec->item_defs.begin();
       it != spec->item_defs.end();
       it++) {
    Item_area* area = &(it->second);
    while (area && area->place_item()) {
      Point p = area->pick_location();
      Item item( area->pick_type() );
      tiles[p.x][p.y].items.push_back(item);
    }
  }
}

void Submap::generate_above(World_terrain* type, Submap* below)
{
  if (!type) {
    debugmsg("Submap::generate_above(NULL, ?) called!");
    generate_empty();
    return;
  }
  if (!below) {
    debugmsg("Submap::generate_above(?, NULL) called!");
    generate_empty();
    return;
  }

  level = below->level + 1;
  subname = below->subname;
  rotation = below->rotation;

  Mapgen_spec* spec = MAPGEN_SPECS.random_with_subname(subname, level);
  if (!spec) {
    generate_open();
    return;
  }
  World_terrain* ter[5];
  ter[0] = type;
  for (int i = 0; i < 5; i++) {
    ter[i] = NULL;
  }
  spec->rotate(rotation);
  spec->prepare(ter, false);  // false means no rotation happens here.
  generate(spec);
// We might need to add stairs to match what's below.
  if (spec->has_flag(MAPFLAG_AUTOSTAIRS)) {
    for (int x = 0; x < SUBMAP_SIZE; x++) {
      for (int y = 0; y < SUBMAP_SIZE; y++) {
        Tile* t = &(below->tiles[x][y]);
        if (t->terrain && t->terrain->has_flag(TF_STAIRS_UP)) {
          std::string stair_name = t->terrain->inverse;
          Terrain* stair = TERRAIN.lookup_name(stair_name);
          if (stair) {
            tiles[x][y].set_terrain(stair);
          }
        }
      }
    }
  }
}

void Submap::clear_items()
{
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      tiles[x][y].items.clear();
    }
  }
}

bool Submap::add_item(Item item, int x, int y)
{
  if (x < 0 || y < 0 || x >= SUBMAP_SIZE || y >= SUBMAP_SIZE) {
    return false;
  }
  if (item.count > 1) {
    int count = item.count;
    item.count = 1;
    for (int i = 0; i < count; i++) {
      if (!add_item(item, x, y)) {
        return false;
      }
    }
    return true;
  }
  if ((tiles[x][y].move_cost() > 0 || tiles[x][y].has_flag(TF_CONTAINER)) &&
      !tiles[x][y].has_flag(TF_NO_ITEMS)) {
    tiles[x][y].items.push_back(item);
  } else {
// Pick a random adjacent space with move_cost != 0
    std::vector<Point> valid_points;
    for (int px = x - 1; px <= x + 1; px++) {
      for (int py = y - 1; py <= y + 1; py++) {
        if (px >= 0 && py >= 0 && px < SUBMAP_SIZE && py < SUBMAP_SIZE &&
            tiles[px][py].move_cost() > 0) {
          valid_points.push_back( Point(px, py) );
        }
      }
    }
    if (valid_points.empty()) {
      return false; // No valid points!  Oh well.  ITEM OBLITERATED
// TODO: Don't obliterate items.
    }
    int index = rng(0, valid_points.size() - 1);
    Point p = valid_points[index];
    tiles[p.x][p.y].items.push_back(item);
  }
  return true;
}

int Submap::item_count(int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE || x < 0 || y >= SUBMAP_SIZE) {
    return 0;
  }
  return tiles[x][y].items.size();
}

std::vector<Item>* Submap::items_at(int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE || x < 0 || y >= SUBMAP_SIZE) {
    return NULL;
  }
  return &(tiles[x][y].items);
}

Point Submap::random_empty_tile()
{
  std::vector<Point> options;
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      if (tiles[x][y].move_cost() > 0) {
        options.push_back( Point(x, y) );
      }
    }
  }

  if (options.empty()) {
    return Point(-1, -1);
  }
  return options[ rng(0, options.size() - 1) ];
}

std::string Submap::get_spec_name()
{
  if (!spec_used) {
    return "Unknown";
  }
  return spec_used->get_name();
}

std::string Submap::get_world_ter_name()
{
  if (!spec_used) {
    return "";
  }
  return spec_used->terrain_name;
}

std::string Submap::save_data()
{
  std::stringstream ret;

  if (spec_used) {
    ret << "Spec: " << spec_used->get_short_name() << std::endl;
  }

  if (!subname.empty()) {
    ret << "Subname: " << subname << std::endl;
  }
  ret << "Rotation: " << int(rotation) << std::endl;
  ret << "Level: " << level << std::endl;
  ret << "Tiles: " << std::endl;
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      ret << tiles[x][y].save_data() << std::endl;
    }
  }

  ret << "Done";

  return ret.str();
}

bool Submap::load_data(std::istream& data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      debugmsg("Couldn't read Submap data.");
      return false;
    }
    ident = no_caps(ident);

    if (ident == "tiles:") {
      for (int x = 0; x < SUBMAP_SIZE; x++) {
        for (int y = 0; y < SUBMAP_SIZE; y++) {
          if (!tiles[x][y].load_data(data)) {
            debugmsg("Failed to read Submap Tile [%d:%d]", x, y);
            return false;
          }
        }
      }

    } else if (ident == "spec:") {
      std::string specname;
      std::getline(data, specname);
      specname = trim(specname);
      spec_used = MAPGEN_SPECS.lookup_name(specname);
      if (!spec_used) {
        debugmsg("Unknown Mapgen_spec %s.", specname.c_str());
        return false;
      }

    } else if (ident == "subname:") {
      std::string tmpname;
      std::getline(data, tmpname);
      subname = trim(tmpname);

    } else if (ident == "rotation:") {
      int tmprot;
      data >> tmprot;
      if (tmprot < 0 || tmprot > DIR_WEST) {
        debugmsg("Invalid rotation %d (range is 0 - 5).", tmprot);
        return false;
      }
      rotation = Direction(tmprot);
      std::getline(data, junk);

    } else if (ident == "level:") {
      data >> level;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown Submap property '%s'", ident.c_str());
    }
  }
  if (ident != "done") {
    debugmsg("Submap save data was incomplete.");
    return false;
  }
  return true;
}

Submap_pool::Submap_pool()
{
  sector = Point(-1, -1);
}

Submap_pool::~Submap_pool()
{
  for (std::list<Submap*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    delete (*it);
  }
}

Submap* Submap_pool::at_location(int x, int y, int z)
{
  return at_location( Tripoint(x, y, z) );
}

Submap* Submap_pool::at_location(Point p)
{
  Tripoint trip(p.x, p.y, 0);
  return at_location(trip);
}

Submap* Submap_pool::at_location(Tripoint p)
{
  if (point_map.count(p) > 0) {
    return point_map[p];
  }
  return generate_submap(p);
}

void Submap_pool::load_area(int sector_x, int sector_y)
{
  int max_sector = WORLDMAP_SIZE / SECTOR_SIZE;
  if (sector_x < 0 || sector_x > max_sector ||
      sector_y < 0 || sector_y > max_sector   ) {
    debugmsg("Submap_pool::load_area(%d, %d) - limit (%d, %d)",
             sector_x, sector_y, max_sector, max_sector);
    return;
  }

// Check if we're loading what we already have - if so, skip all this work
  if (sector_x == sector.x && sector_y == sector.y) {
    return;
  }

// Start by clearing out existing submaps which we don't need...
// (unless we're brand-new)
  if (sector.x != -1 && sector.y != -1) {
    clear_submaps(sector_x, sector_y);
  }

/* At this point, we've saved and deleted all submaps which won't be in the
 * updated pool.  The next step is to load (or generate if need be) all the
 * submaps which WILL be in the updated pool.
 */
  init_submaps(sector_x, sector_y);

// Finally, set sector.
  sector = Point(sector_x, sector_y);

}

void Submap_pool::load_area_centered_on(int center_x, int center_y)
{
  if (center_x < 0 || center_x >= WORLDMAP_SIZE ||
      center_y < 0 || center_y >= WORLDMAP_SIZE   ) {
    debugmsg("Submap_pool::load_area_centered_on(%d, %d) - limit (%d, %d)",
             center_x, center_y, WORLDMAP_SIZE, WORLDMAP_SIZE);
    return;
  }
// e.g. SECTOR_SIZE = 10; 47 => 40
  int sector_x = center_x - (center_x % SECTOR_SIZE);
  int sector_y = center_y - (center_y % SECTOR_SIZE);

// 40 => 4
  sector_x /= SECTOR_SIZE;
  sector_y /= SECTOR_SIZE;

// 4 => 3
  sector_x--;
  sector_y--;

  load_area(sector_x, sector_y);
}

int Submap_pool::size()
{
  return instances.size();
}

void Submap_pool::remove_point(Tripoint p)
{
  if (point_map.count(p) == 0) {
    if (TESTING_MODE) {
      debugmsg("Submap_pool couldn't remove point %s.", p.str().c_str());
    }
    return;
  }
  point_map.erase(p);
}

void Submap_pool::remove_submap(Submap* sm)
{
  if (!sm) {
    if (TESTING_MODE) {
      debugmsg("Submap_pool couldn't remove submap %d.", sm);
    }
    return;
  }
  delete sm;
  instances.remove(sm);
}

void Submap_pool::clear_submaps(int sector_x, int sector_y)
{
  std::string map_dir = SAVE_DIR + "/" + GAME.worldmap->get_name();
  if (!directory_exists(map_dir)) {
    if (!create_directory(map_dir)) {
      debugmsg("Couldn't create directory '%s'.", map_dir.c_str());
      return;
    }
  }

  int num_removed = 0;
  for (int sx = sector.x; sx < sector.x + 3; sx++) {
    for (int sy = sector.y; sy < sector.y + 3; sy++) {
// Only save sectors that won't exist in the new Submap_pool.
      if (sx < sector_x || sx >= sector_x + 3 ||
          sy < sector_y || sy >= sector_y + 3   ) {
        std::stringstream filename;
        filename << map_dir << "/map." << sx << "." << sy;
        std::ofstream fout;
        fout.open( filename.str().c_str() );
        if (!fout.is_open()) {
          debugmsg("Couldn't open '%s' for writing.", filename.str().c_str());
          return;
        }
  
        int start_x = sx * SECTOR_SIZE, start_y = sy * SECTOR_SIZE;
        for (int mx = start_x; mx < start_x + SECTOR_SIZE; mx++) {
          for (int my = start_y; my < start_y + SECTOR_SIZE; my++) {
            Tripoint curpos = Tripoint(mx, my, 0);
// while loop moves upwards until we stop having maps
            while (point_map.count(curpos) > 0) {
              Submap* curmap = point_map[curpos];
              fout << curpos.x << " " << curpos.y << " " << curpos.z << " " <<
                      curmap->save_data() << std::endl;
              remove_point(curpos);
              remove_submap(curmap);
              num_removed++;
              curpos.z++;
            }
          } // for (start_y <= mx < start_x + SECTOR_SIZE
        } // for (start_x <= mx < start_x + SECTOR_SIZE
      } // If <sector is moving out of bounds>
    } // for (int sy = sector.y; sy < sector.y + 3; sy++)
  } // for (int sx = sector.x; sx < sector.x + 3; sx++)

  if (TESTING_MODE) {
    debugmsg("%d submaps erased; %d left (point_map %d, instances %d).",
             num_removed, size(), point_map.size(), instances.size());
  }

}

void Submap_pool::init_submaps(int sector_x, int sector_y)
{
  std::string map_dir = SAVE_DIR + "/" + GAME.worldmap->get_name();
/* The first time we use a new world, the directory won't even exist!  This will
 * be remedied the first time we have to SAVE Submaps, but for now, we'll take
 * it as a sign that we need to generate ALL of them.
 */
  bool gen_all = false;
  if (!directory_exists(map_dir)) {
    gen_all = true;
  }
  for (int sx = sector_x; sx < sector_x + 3; sx++) {
    for (int sy = sector_y; sy < sector_y + 3; sy++) {
/* Again, we check for overlap with the *old* position - no need to re-load
 * or re-generate those submaps.
 */
      if (sx < sector.x || sx >= sector.x + 3 ||
          sy < sector.y || sy >= sector.y + 3   ) {
// Attempt to load from file
        std::stringstream filename;
        if (!gen_all) {
          filename << SAVE_DIR << "/" << GAME.worldmap->get_name() << "/map." <<
                      sx << "." << sy;
        }
        if (gen_all || !load_submaps( filename.str() )) {
// No file!  Generate the submaps.
          int startx = sx * SECTOR_SIZE, starty = sy * SECTOR_SIZE;
          for (int mx = startx; mx < startx + SECTOR_SIZE; mx++) {
            for (int my = starty; my < starty + SECTOR_SIZE; my++) {
              generate_submap(mx, my);
            }
          }
        }
      }
    }
  }
}

bool Submap_pool::load_submaps(std::string filename)
{
  std::ifstream fin;
  fin.open( filename.c_str() );
  if (!fin.is_open()) {
    return false;
  }
  while (!fin.eof()) {
    Tripoint smpos;
    fin >> smpos.x >> smpos.y >> smpos.z;
    if (!fin.eof()) {
/*
      if (TESTING_MODE) {
        debugmsg("Loading %s...", smpos.str().c_str());
      }
  */
      bool use_sm = true;
      if (point_map.count(smpos) > 0) {
        use_sm = false;
  /*
        debugmsg("Submap_pool collision at %s!", smpos.str().c_str());
        return false;
  */
      }
      Submap* sm = new Submap;
      if (sm->load_data(fin)) {
        if (use_sm) {
          instances.push_back(sm);
          point_map[smpos] = sm;
        } else {
          delete sm;
        }
      } else {
        delete sm;
        debugmsg("Failed to load submap at %s.", smpos.str().c_str());
        return false;
      }
    }
  }
  return true;
}

Submap* Submap_pool::generate_submap(int x, int y, int z)
{
  return generate_submap( Tripoint(x, y, z) );
}

Submap* Submap_pool::generate_submap(Tripoint p)
{
  Submap* sub = new Submap;
  if (p.z > 0) {
    Submap* below = at_location(p.x, p.y, p.z - 1);
    Worldmap_tile *tile = GAME.worldmap->get_tile(p.x, p.y);
    if (!tile) {
      sub->generate_empty();
      return sub;
    }
    sub->generate_above(tile->terrain, below);
    return sub;
  }
  sub->generate(GAME.worldmap, p.x, p.y, p.z);
  point_map[p] = sub;
  instances.push_back(sub);
  return sub;
}

Map::Map()
{
  posx = 0;
  posy = 0;
  posz = 0;

  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      for (int z = 0; z < VERTICAL_MAP_SIZE * 2 + 1; z++) {
        submaps[x][y][z] = NULL;
      }
    }
  }
}

Map::~Map()
{
}

void Map::generate_empty()
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      submaps[x][y][posz]->generate_empty();
    }
  }
}

/*
void Map::test_generate(std::string terrain_name)
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      submaps[x][y][posz]->generate(terrain_name);
    }
  }
}
*/

void Map::generate(Worldmap *world, int wposx, int wposy, int wposz)
{
// All arguments default to -999
  if (wposx != -999) {
    posx = wposx;
  }
  if (wposy != -999) {
    posy = wposy;
  }
  if (wposz != -999) {
    posz = wposz;
  }
// TODO: Support posz < 0
  for (int z = 0; z <= posz + 1; z++) {
    int z_index = z + VERTICAL_MAP_SIZE - posz;
    for (int x = 0; x < MAP_SIZE; x++) {
      for (int y = 0; y < MAP_SIZE; y++) {
/* at_location either returns the existing submap with that location keyed in,
 * or creates a new submap, generates it with the world information at that
 * location, and returns it.
 */
        submaps[x][y][z_index] = SUBMAP_POOL.at_location(posx + x, posy + y, z);
        if (z == 0) {
          spawn_monsters(world, posx + x, posy + y, x, y, z);
        }
      }
    }
  }
}

void Map::shift(Worldmap *world, int shiftx, int shifty, int shiftz)
{
  if (shiftx == 0 && shifty == 0 && shiftz == 0) {
    return;
  }
  posx += shiftx;
  posy += shifty;
  posz += shiftz;
  generate(world);
}

void Map::spawn_monsters(Worldmap *world, int worldx, int worldy,
                         int subx, int suby, int zlevel)
{
// If we have bad inputs, return with an error message
  if (!world) {
    debugmsg("Map::spawn_monsters() called with NULL world");
    return;
  }
  if (subx < 0 || suby < 0 || subx >= MAP_SIZE || suby >= MAP_SIZE) {
    debugmsg("Map::spawn_monsters() called on submap [%d:%d]", subx, suby);
    return;
  }
// Fetch the monsters from the world
  std::vector<Monster_spawn>* monsters = world->get_spawns(worldx, worldy);

  if (monsters->empty()) {
    return; // No monsters here, skip the rest!
  }

/*
  int zdex = zlevel + VERTICAL_MAP_SIZE - posz;
debugmsg("Spawning monsters at World[%d:%d](%s), Submap[%d:%d:%d](%s)", 
         worldx, worldy, world->get_name(worldx, worldy).c_str(),
         subx, suby, posz, submaps[subx][suby][zdex]->get_spec_name().c_str());
*/

// Pick some empty tiles
/* TODO: Support placing aquatic monsters in water tiles, etc.
 *       Perhaps by replacing random_empty_tile() with tile_for(Monster_type*)?
 */
  int minx = subx * SUBMAP_SIZE, miny = suby * SUBMAP_SIZE;
  int maxx = minx + SUBMAP_SIZE - 1, maxy = miny + SUBMAP_SIZE - 1;
/*
  debugmsg("Submap [%d:%d:%d], tiles [%d:%d] to [%d:%d]",
           subx, suby, zlevel, minx, miny, maxx, maxy);
*/
  std::vector<Point> available_tiles;
  for (int x = minx; x <= maxx; x++) {
    for (int y = miny; y <= maxy; y++) {
      if (move_cost(x, y, zlevel) > 0) {
        available_tiles.push_back( Point(x, y) );
      }
    }
  }

/*
  if (available_tiles.empty()) {
    debugmsg("No available tiles!");
    return;
  }
*/
  for (int i = 0; !available_tiles.empty() && i < monsters->size(); i++) {
    while (!available_tiles.empty() && (*monsters)[i].population > 0) {
// Pick an available tile and remove it from the list
      int index = rng(0, available_tiles.size() - 1);
      Point pos = available_tiles[index];
      available_tiles.erase( available_tiles.begin() + index );
// Create a monster and place it there
      Monster* mon = (*monsters)[i].generate_monster();
//debugmsg("Generating '%s'", mon->get_name().c_str());
      mon->pos = Tripoint(pos.x, pos.y, zlevel);
/*
debugmsg("Placed at [%d:%d:%d] - '%s'", pos.x, pos.y, posz,
         get_name(pos.x, pos.y, posz).c_str());
*/
      GAME.entities.add_entity(mon);
      (*monsters)[i].population--;
    }
  }
}

Generic_map Map::get_movement_map(Entity_AI AI,
                                  Tripoint origin, Tripoint target)
{
// Set the bounds of the map
  int min_x = (origin.x < target.x ? origin.x : target.x);
  int min_y = (origin.y < target.y ? origin.y : target.y);
  int min_z = (origin.z < target.z ? origin.z : target.z);
  int max_x = (origin.x > target.x ? origin.x : target.x);
  int max_y = (origin.y > target.y ? origin.y : target.y);
  int max_z = (origin.z > target.z ? origin.z : target.z);

// Expand the bounds of the map by our area awareness bonus.
  min_x -= AI.area_awareness;
  min_y -= AI.area_awareness;
  max_x += AI.area_awareness;
  max_y += AI.area_awareness;

  int x_size = 1 + max_x - min_x;
  int y_size = 1 + max_y - min_y;
  int z_size = 1 + max_z - min_z;

  Generic_map ret(x_size, y_size, z_size);
  ret.x_offset = min_x;
  ret.y_offset = min_y;
  ret.z_offset = min_z;

  for (int x = min_x; x <= max_x; x++) {
    for (int y = min_y; y <= max_y; y++) {
      for (int z = min_z; z <= max_z; z++) {
        int map_x = x - min_x;
        int map_y = y - min_y;
        int map_z = z - min_z;
        int cost = move_cost(x, y, z);
// TODO: If there's a field here, increase cost accordingly
        if (cost == 0 && is_smashable(x, y, z)) {
          cost = 500; // TODO: Estimate costs more intelligently
        }
        ret.set_cost(map_x, map_y, map_z, cost);
        if (has_flag(TF_STAIRS_UP, x, y, z)) {
          ret.set_goes_up( Tripoint(map_x, map_y, map_z) );
        }
        if (has_flag(TF_STAIRS_DOWN, x, y, z)) {
          ret.set_goes_down( Tripoint(map_x, map_y, map_z) );
        }
      }
    }
  }

  return ret;
}

Generic_map Map::get_dijkstra_map(Tripoint target, int weight,
                                  bool include_smashable)
{
  Generic_map ret(SUBMAP_SIZE * MAP_SIZE, SUBMAP_SIZE * MAP_SIZE, posz + 1);
  ret.set_cost(target, weight);
  std::vector<Tripoint> active;
  active.push_back(target);
  while (!active.empty()) {
    Tripoint cur = active[0];
    active.erase(active.begin());
// Check all adjacent terrain
    for (int x = cur.x - 1; x <= cur.x + 1; x++) {
      for (int y = cur.y - 1; y <= cur.y + 1; y++) {
        if (x == cur.x && y == cur.y) { // Skip our own cell
          y++;
        }
        if (((include_smashable && is_smashable(x, y, cur.z)) ||
             move_cost(x, y, cur.z) > 0) &&
            ret.get_cost(x, y, cur.z) < ret.get_cost(cur) - 1) {
          ret.set_cost(x, y, cur.z, ret.get_cost(cur) - 1);
          active.push_back( Tripoint(x, y, cur.z) );
        }
      }
    }
    if (has_flag(TF_STAIRS_DOWN, cur)) {
      Tripoint down(cur.x, cur.y, cur.z - 1);
      if (ret.get_cost(down) < ret.get_cost(cur) - 1) {
        ret.set_cost(down, ret.get_cost(cur) - 1);
        active.push_back( down );
      }
    }
    if (has_flag(TF_STAIRS_UP, cur)) {
      Tripoint down(cur.x, cur.y, cur.z + 1);
      if (ret.get_cost(down) < ret.get_cost(cur) - 1) {
        ret.set_cost(down, ret.get_cost(cur) - 1);
        active.push_back( down );
      }
    }
  } // while (!active.empty())
  return ret;
}

int Map::move_cost(Tripoint pos)
{
  return move_cost(pos.x, pos.y, pos.z);
}

int Map::move_cost(int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  if (!t) {
    return 100;
  }
  return t->move_cost();
}

int Map::get_height(Tripoint pos)
{
  return get_height(pos.x, pos.y, pos.z);
}

int Map::get_height(int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  if (!t) {
    return 0;
  }
  return t->get_height();
}

bool Map::is_smashable(Tripoint pos)
{
  return is_smashable(pos.x, pos.y, pos.z);
}

bool Map::is_smashable(int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  return (t && t->is_smashable());
}

bool Map::has_flag(Terrain_flag flag, Tripoint pos)
{
  return has_flag(flag, pos.x, pos.y, pos.z);
}

bool Map::has_flag(Terrain_flag flag, int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  return (t && t->has_flag(flag));
}

bool Map::blocks_sense(Sense_type sense, Tripoint pos, int z_value)
{
  Tile *t = get_tile(pos);
  return (t && t->blocks_sense(sense, z_value));
}

bool Map::blocks_sense(Sense_type sense, int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  return (t && t->blocks_sense(sense));
}

bool Map::add_item(Item item, Tripoint pos)
{
  return add_item(item, pos.x, pos.y, pos.z);
}

bool Map::add_item(Item item, int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = posz;
  }
  z = z - posz + VERTICAL_MAP_SIZE;
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE ||
      z < 0 || z >= VERTICAL_MAP_SIZE * 2 + 1) {
    return false;
  }
  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  x %= SUBMAP_SIZE;
  y %= SUBMAP_SIZE;
  return submaps[sx][sy][z]->add_item(item, x, y);
}

void Map::clear_items()
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      submaps[x][y][VERTICAL_MAP_SIZE]->clear_items();
    }
  }
}

bool Map::remove_item(Item* it, int uid)
{
// Sanity check
  if (it == NULL && uid < 0) {
    return false;
  }
// Code duplication from find_item(), but what can ya do
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      for (int z = 0; z < VERTICAL_MAP_SIZE * 2 + 1; z++) {
        Submap* sm = submaps[x][y][z];
        if (sm) {
          for (int sx = 0; sx < SUBMAP_SIZE; sx++) {
            for (int sy = 0; sy < SUBMAP_SIZE; sy++) {
              std::vector<Item>* items = sm->items_at(sx, sy);
              if (!items) {
                debugmsg("NULL Items in Map::find_item_uid()");
              }
              for (int i = 0; i < items->size(); i++) {
                if ( &( (*items)[i] ) == it || (*items)[i].get_uid() == uid ) {
                  items->erase( items->begin() + i );
                  return true;
                }
              }
            }
          }
        }
      }
    }
  }
// If we never found it, return false
  return false;
}

bool Map::remove_item_uid(int uid)
{
  return remove_item(NULL, uid);
}

bool Map::add_field(Field_type* type, Tripoint pos, std::string creator)
{
  return add_field(type, pos.x, pos.y, pos.z);
}

bool Map::add_field(Field_type* type, int x, int y, int z, std::string creator)
{
  if (!type) {
    debugmsg("Tried to add NULL field! (%s)", creator.c_str());
    return false;
  }
  Field field(type, 0, creator);
  return add_field(field, x, y, z);
}

bool Map::add_field(Field field, Tripoint pos)
{
  return add_field(field, pos.x, pos.y, pos.z);
}

bool Map::add_field(Field field, int x, int y, int z)
{
  Tile* tile = get_tile(x, y, z);
  if (tile->has_field()) {
// We can combine fields of the same type
    tile->field += field;
    field_points.push_back( Tripoint(x, y, (z == 999 ? posz : z)) );
    return true;
  }
  if (tile->move_cost() == 0 && !field.has_flag(FIELD_FLAG_SOLID)) {
    return false;
  }
  tile->field = field;
  field_points.push_back( Tripoint(x, y, (z == 999 ? posz : z)) );
  return true;
}

int Map::item_count(Tripoint pos)
{
  return item_count(pos.x, pos.y, pos.z);
}

int Map::item_count(int x, int y, int z)
{
  std::vector<Item>* it = items_at(x, y, z);
  if (!it) {
    return 0;
  }
  return it->size();
}

std::vector<Item>* Map::items_at(Tripoint pos)
{
  return items_at(pos.x, pos.y, pos.z);
}

std::vector<Item>* Map::items_at(int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = posz;
  }
  z = z - posz + VERTICAL_MAP_SIZE;
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE ||
      z < 0 || z >= VERTICAL_MAP_SIZE * 2 + 1) {
    return NULL;
  }
  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  x %= SUBMAP_SIZE;
  y %= SUBMAP_SIZE;
  return submaps[sx][sy][z]->items_at(x, y);
}

Furniture* Map::furniture_at(Tripoint pos)
{
  return furniture_at(pos.x, pos.y, pos.z);
}

Furniture* Map::furniture_at(int x, int y, int z)
{
  Tile* tile = get_tile(x, y, z);
  if (!tile) {
    return NULL;
  }
  if (!tile->furniture.is_real()) {
    return NULL;
  }
  return &(tile->furniture);
}

void Map::add_furniture(Furniture furn, Tripoint pos)
{
  add_furniture(furn, pos.x, pos.y, pos.z);
}

void Map::add_furniture(Furniture furn, int x, int y, int z)
{
  Tile* tile = get_tile(x, y, z);
  if (!tile) {
    return;
  }
  tile->add_furniture(furn);
}

/* grab_furniture() returns a list of the furniture at (target), along with any
 * furniture connected to it.  Furniture is considered connected if it touches
 * in an orthogonal direction, and has the same UID from the mapgen file.
 * To achieve this, grab_furniture() recurses into orthogonal tiles, and stops
 * if there's no furniture there, or if the furniture is of a different UID.
 * (id) defaults to -1, but is set when we recurse.
 * Checked is a vector of points already checked, so that we don't get stuck in
 * an infinite loop, cycling between two tiles.  If it's NULL, we'll set it and
 * delete it before we exit.
 */
std::vector<Furniture_pos> Map::grab_furniture(Tripoint origin, Tripoint target,
                                               int id,
                                               std::vector<Tripoint>* checked)
{
/* We have to create (checked) if it doesn't exist.  But we also have to
 * *remember* that we created it, and delete it before returning, to avoid
 * memory leaks.
 */
  bool created_checked = false;
  if (checked == NULL) {
    created_checked = true;
    checked = new std::vector<Tripoint>;
  }
  std::vector<Furniture_pos> ret;
  Furniture* grabbed = furniture_at(target);
// Return an empty vector if no furniture there
  if (!grabbed) {
    if (created_checked) {
      delete checked;
    }
    return ret;
  }
// Return an empty vector if furniture is a different UID
  if (id >= 0 && grabbed->uid != id) {
    if (created_checked) {
      delete checked;
    }
    return ret;
  }
// Success!  Push back the furniture at the target tile.
  Furniture_pos at_grab;
  at_grab.furniture = *grabbed;
  at_grab.pos = Point(target.x - origin.x, target.y - origin.y);
  ret.push_back(at_grab);

// Now recurse...
  checked->push_back(target); // Ensure we won't try this target again.
  int id_used = grabbed->uid;
  for (int i = 1; i <= 4; i++) {
    Tripoint next = target;
    switch (i) {
      case 1: next.x++; break;
      case 2: next.x--; break;
      case 3: next.y++; break;
      case 4: next.y--; break;
    }
    bool next_okay = (checked != NULL); // If checked is somehow NULL, skip this
    for (int n = 0; next_okay && n < checked->size(); n++) {
      if ( (*checked)[n] == next ) {
        next_okay = false;
      }
    }
    if (next_okay) {
      std::vector<Furniture_pos> adj = grab_furniture(origin, next, id_used,
                                                      checked);
      for (int n = 0; n < adj.size(); n++) {
        ret.push_back(adj[n]);
      }
    }
  }
  if (created_checked) {
    delete checked;
  }
  return ret;
}

void Map::clear_furniture(Tripoint pos)
{
  clear_furniture(pos.x, pos.y, pos.z);
}

void Map::clear_furniture(int x, int y, int z)
{
  Tile* tile = get_tile(x, y, z);
  if (tile) {
    tile->remove_furniture();
  }
}

bool Map::contains_field(Tripoint pos)
{
  return contains_field(pos.x, pos.y, pos.z);
}

bool Map::contains_field(int x, int y, int z)
{
  return (get_tile(x, y, z)->has_field());
}

Field* Map::field_at(Tripoint pos)
{
  return field_at(pos.x, pos.y, pos.z);
}

Field* Map::field_at(int x, int y, int z)
{
  Tile* tile = get_tile(x, y, z);
  return &(tile->field);
}

int Map::field_uid_at(Tripoint pos)
{
  return field_uid_at(pos.x, pos.y, pos.z);
}

int Map::field_uid_at(int x, int y, int z)
{
  Field* tmp = field_at(x, y, z);
  if (tmp->level <= 0) {
    return -1;
  }
  return tmp->get_type_uid();
}

Tile* Map::get_tile(Tripoint pos)
{
  return get_tile(pos.x, pos.y, pos.z);
}

Tile* Map::get_tile(int x, int y, int z)
{
// TODO: Set all fields, traps, etc. on tile_oob to "nothing"
  if (z == 999) { // z defaults to 999
    z = posz;
  }
  z = z - posz + VERTICAL_MAP_SIZE;
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE ||
      z < 0 || z >= VERTICAL_MAP_SIZE * 2 + 1 ) {
    tile_oob.set_terrain(TERRAIN.lookup_uid(0));
    tile_oob.field.dead = true;
    return &tile_oob;
  }

  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  if (submaps[sx][sy][z] == NULL) {
    return NULL;
  }
  return &(submaps[sx][sy][z]->tiles[x % SUBMAP_SIZE][y % SUBMAP_SIZE]);
}

std::string Map::get_name(Tripoint pos)
{
  return get_name(pos.x, pos.y, pos.z);
}

std::string Map::get_name(int x, int y, int z)
{
  Tile* t = get_tile(x, y, z);
  if (!t) {
    return "Bug - NULL tile";
  }
  return t->get_name();
}

std::string Map::get_name_indefinite(Tripoint pos)
{
  return get_name_indefinite(pos.x, pos.y, pos.z);
}

std::string Map::get_name_indefinite(int x, int y, int z)
{
  Tile* t = get_tile(x, y, z);
  if (!t) {
    return "Bug - NULL tile";
  }
  return t->get_name_indefinite();
}

void Map::smash(int x, int y, Damage_set dam, bool make_sound)
{
  return smash(x, y, 999, dam, make_sound);
}

void Map::smash(int x, int y, int z, Damage_set dam, bool make_sound)
{
  Tile* hit = get_tile(x, y, z);
  if (hit) {
    std::string sound = hit->smash(dam);
    if (make_sound) {
      GAME.make_sound(sound, x, y);
    }
  }
}

void Map::smash(Tripoint pos, Damage_set dam, bool make_sound)
{
  return smash(pos.x, pos.y, pos.z, dam);
}

void Map::damage(int x, int y, Damage_set dam)
{
  damage(x, y, 999, dam);
}

void Map::damage(int x, int y, int z, Damage_set dam)
{
  Tile* hit = get_tile(x, y, z);
  if (hit) {
    bool may_explode = has_flag(TF_EXPLOSIVE, x, y, z);
    std::string old_name = get_name(x, y, z);
    if (hit->damage(dam) && may_explode) {
// If we were explosive, then destroying us sets off an explosion!
// TODO: Shoudl explosion particulars be drawn from data?  Probably...
      Explosion expl;
      expl.radius = Dice(2, 2, 5);  // Average 8
      expl.force  = Dice(4, 6, 16); // Average 30
      expl.shrapnel_count  = Dice(2, 6, -2); // Average 5
      expl.shrapnel_damage = Dice(3, 6,  4); // Average 14.5
      expl.field_name = "fire";
      expl.field_chance = 25;
      expl.field_duration = Dice(50, 10, 200); // Average 475
      std::stringstream expl_reason;
      expl_reason << "an exploding " << old_name;
      expl.reason = expl_reason.str();
      expl.explode( Tripoint(x, y, z) );
    }
  }
}

void Map::damage(Tripoint pos, Damage_set dam)
{
  damage(pos.x, pos.y, pos.z, dam);
}

bool Map::apply_signal(std::string signal, Tripoint pos, Entity* user)
{
  return apply_signal(signal, pos.x, pos.y, pos.z, user);
}

bool Map::apply_signal(std::string signal, int x, int y, int z, Entity* user)
{
  Tile* target = get_tile(x, y, z);
  if (target->signal_applies(signal)) {
    target->apply_signal(signal, user);
    return true;
  }
  return false;
}

/* TODO:  We should track currently-active fields in a list of points.  At
 *        present, we check *all* tiles for an active field.  This is probably
 *        inefficient.
 */
void Map::process_fields()
{
/* TODO:  Won't work below ground level.
 * TODO:  Since we start at the upper-left and work our way down & right, fields
 *        to the north-west will have a better chance of spreading than fields
 *        to the south-east.  Best way to fix this is to create a output map of
 *        fields, the copy that output map back to this after processing is
 *        done.
 */
  for (int i = 0; i < field_points.size(); i++) {
    Tripoint pos = field_points[i];
    Field* field = field_at(pos);
    if (!field) {
      debugmsg("Somehow encountered NULL field at %s!", pos.str().c_str());
      return;
    }
    if (field->is_valid()) {
      Entity* ent = GAME.entities.entity_at(pos);
      if (ent) {
        field->hit_entity(ent);
      }
      field->process(this, pos);
      if (!field->is_valid()) { // It was destroyed/extinguished!
        field_points.erase( field_points.begin() + i);
        i--;
      }
    }
  }
}

/* Still using Cataclysm/DDA style LOS.  It sucks and is slow and I hate it.
 * Basically, iterate over all Bresenham lines between [x0,y0] and [x1,y1].
 * If any of the lines doesn't have something that blocks the relevent sense,
 * return true.  If we iterate through all of them and they all block, return
 * false.
 */
bool Map::senses(int x0, int y0, int x1, int y1, int range, Sense_type sense)
{
  return senses(x0, x0, posz, x1, y1, posz, range, sense);
}

bool Map::senses(int x0, int y0, int z0, int x1, int y1, int z1, int range,
                 Sense_type sense)
{
  if (x0 < 0 || y0 < 0 ||
      x1 >= SUBMAP_SIZE * MAP_SIZE || y1 >= SUBMAP_SIZE * MAP_SIZE) {
    return false;
  }
  if (range >= 0 && rl_dist(x0, y0, z0, x1, y1, z1) > range) {
    return false;
  }
  if (sense == SENSE_SIGHT) {
    std::vector<Tripoint> line = line_of_sight(x0, y0, z0, x1, y1, z1);
    return (!line.empty() && (range < 0 || line.size() <= range));
  } else if (sense == SENSE_SMELL) {
// TODO: More realistic smell
    return (rl_dist(x0, y0, z0, x1, y1, z1) <= range);
  }
  return false;
}

bool Map::senses(Point origin, Point target, int range, Sense_type sense)
{
  return senses(origin.x, origin.y, posz, target.x, target.y, posz, range,
                sense);
}

bool Map::senses(Tripoint origin, Tripoint target, int range, Sense_type sense)
{
  return senses(origin.x, origin.y, origin.z, target.x, target.y, target.z,
                range, sense);
}

std::vector<Tripoint> Map::line_of_sight(int x0, int y0, int x1, int y1)
{
  return line_of_sight(x0, y0, posz, x1, y1, posz);
}

std::vector<Tripoint> Map::line_of_sight(int x0, int y0, int z0,
                                         int x1, int y1, int z1)
{
  std::vector<Tripoint>  lines;    // Process many lines at once.
  std::vector<std::vector<Tripoint> > return_values;
  std::vector<int>    t_values; // T-values for Bresenham lines

  int dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
  int ax = abs(dx) << 1, ay = abs(dy) << 1;
  int sx = (dx < 0 ? -1 : 1), sy = (dy < 0 ? -1 : 1);
  int dist = rl_dist(x0, y0, x1, y1);
  int z_step;
  if (dist == 0) {
    z_step = 0;
  } else {
    z_step = (100 * dz) / dist;
  }
  if (dx == 0) {
    sx = 0;
  }
  if (dy == 0) {
    sy = 0;
  }

  int min_t = (ax > ay ? ay - ax : ax - ay),
      max_t = 0;
  if (dx == 0 || dy == 0) {
    min_t = 0;
  }
// Init our "lines"
  std::vector<Tripoint> seed;
  for (int t = min_t; t <= max_t; t++) {
    lines.push_back( Tripoint(x0, y0, z0) );
    return_values.push_back(seed);
    t_values.push_back(t);
  }
  int z_value = 50; // Each tile is 100 microunits tall, start halfway up
  int z_level = z0;
// Keep going as long as we've got at least one valid line
  while (!lines.empty()) {
// Since we track z_value universally, don't do it inside the for loop below
    bool z_stepped = false;
    int old_z = z_level;
    z_value += z_step;
    if (z_value < 0) {
      z_level--;
      z_value += 100;
      z_stepped = true;
    } else if (z_value >= 100) {
      z_level++;
      z_value -= 100;
      z_stepped = true;
    }
    for (int i = 0; i < lines.size(); i++) {
      lines[i].z = z_level;
      if (ax > ay) {
        lines[i].x += sx;
        if (t_values[i] >= 0) {
          lines[i].y += sy;
          t_values[i] -= ax;
        }
        t_values[i] += ay;
      } else {
        lines[i].y += sy;
        if (t_values[i] >= 0) {
          lines[i].x += sx;
          t_values[i] -= ay;
        }
        t_values[i] += ax;
      }
      return_values[i].push_back(lines[i]);
// Don't need to check z, right?
      if (lines[i].x == x1 && lines[i].y == y1) {
        return return_values[i];
      }
      if (blocks_sense(SENSE_SIGHT, lines[i], z_value) ||
          (z_stepped &&
           blocks_sense(SENSE_SIGHT, lines[i].x, lines[i].y, old_z))) {
        lines.erase(lines.begin() + i);
        t_values.erase(t_values.begin() + i);
        return_values.erase(return_values.begin() + i);
        i--;
      }
    }
  }
  return std::vector<Tripoint>();
}

std::vector<Tripoint> Map::line_of_sight(Point origin, Point target)
{
  return line_of_sight(origin.x, origin.y, target.x, target.y);
}

std::vector<Tripoint> Map::line_of_sight(Tripoint origin, Tripoint target)
{
  return line_of_sight(origin.x, origin.y, origin.z,
                       target.x, target.y, target.z);
}

void Map::draw(Window* w, Entity_pool *entities, Tripoint ref,
               int range, Sense_type sense)
{
  draw(w, entities, ref.x, ref.y, ref.z, range, sense);
}

void Map::draw(Window* w, Entity_pool *entities, int refx, int refy, int refz,
               int range, Sense_type sense)
{
  if (!w) {
    return;
  }
  int winx = w->sizex(), winy = w->sizey();
  if (winy % 2 == 0) {
    winy--; // Only odd numbers are allowed!
  }
  int minx = refx - (winx / 2), maxx = refx + ( (winx - 1) / 2 );
  int miny = refy - (winy / 2) - 1, maxy = refy + ( (winy - 1) / 2 );
  draw_area(w, entities, refx, refy, refz, minx, miny, maxx, maxy, range,
            sense);
}

void Map::draw_area(Window *w, Entity_pool *entities, Tripoint ref,
                    int minx, int miny, int maxx, int maxy,
                    int range, Sense_type sense)
{
  draw_area(w, entities, ref.x, ref.y, ref.z, minx, miny, maxx, maxy, range,
            sense);
}
 
void Map::draw_area(Window *w, Entity_pool *entities,
                    int refx, int refy, int refz,
                    int minx, int miny, int maxx, int maxy,
                    int range, Sense_type sense)
{
  if (!w) {
    return;
  }

// Range defaults to -1; which means use light level
  if (range == -1) {
    range = GAME.get_light_level();
  }

  int winx = w->sizex(), winy = w->sizey();
  int dist = winx > winy ? winx / 2 : winy / 2;
  for (int x = 0; x < winx; x++) {
    for (int y = 0; y < winy; y++) {
      int terx = refx + x - (winx / 2), tery = refy + y - (winy / 2);
      int z_used = posz;
      while (z_used > 0 && has_flag(TF_OPEN_SPACE, terx, tery, z_used)) {
        z_used--;
      }
      int range_used = (dist < range ? dist : range);
      if (senses(refx, refy, refz, terx, tery, z_used, range_used, sense)) {
// If we're inbounds, draw normally...
        if (terx >= minx && terx <= maxx && tery >= miny && tery <= maxy) {
          draw_tile(w, entities, terx, tery, refx, refy, false);
        } else {  // Otherwise, that last "true" means "change colors to dkgray"
          draw_tile(w, entities, terx, tery, refx, refy, false, true);
        }
      } else {
// TODO: Don't use a literal glyph!  TILES GEEZE
        w->putglyph(x, y, glyph(' ', c_black, c_black));
      }
    }
  }
}

void Map::draw_tile(Window* w, Entity_pool *entities, int tilex, int tiley,
                    int refx, int refy, bool invert, bool gray)
{
  draw_tile(w, entities, tilex, tiley, posz, refx, refy, invert, gray);
}

void Map::draw_tile(Window* w, Entity_pool *entities,
                    int tilex, int tiley, int tilez,
                    int refx, int refy, bool invert, bool gray)
{
  if (!w) {
    return;
  }
  int winx = w->sizex(), winy = w->sizey();
  int centerx = winx / 2, centery = winy / 2;
  int dx = tilex - refx, dy = tiley - refy;
  int tile_winx = centerx + dx, tile_winy = centery + dy;
  if (tile_winx < 0 || tile_winx >= winx || tile_winy < 0 || tile_winy >= winy){
    return; // It won't fit in the window!
  }
// Now pick a glyph...
  glyph output;
  bool picked_glyph = false;
  int curz = tilez;
/* Start from the z-level that we're looking at.  As long as there's no entity,
 * and the terrain is open space, drop down a level.
 */
  while (!picked_glyph && curz >= 0) {
    if (entities) {
      Entity* ent = entities->entity_at(tilex, tiley, curz);
      if (ent) {
        output = ent->get_glyph();
        picked_glyph = true;
      }
    }
    if (!picked_glyph) {
      Tile* tile = get_tile(tilex, tiley, curz);
      if (!tile->has_flag(TF_OPEN_SPACE)) {
        output = tile->top_glyph();
        picked_glyph = true;
      }
    }
    if (picked_glyph) {
      if (curz < tilez) {
        output = output.hilite();
      }
    } else {
      curz--;
    }
  }
  if (!picked_glyph) {
    int smx = tilex / SUBMAP_SIZE, smy = tiley / SUBMAP_SIZE;
    if (smx < 0 || smx >= MAP_SIZE || smy < 0 || smy >= MAP_SIZE) {
      debugmsg("Could not find a glyph - out of bounds!");
    } else {
// Find the submap the tile's in...
      int smz = tilez - posz + VERTICAL_MAP_SIZE;
      Submap* sm = submaps[smx][smy][smz];
      while (!sm && smz > 0) {
        smz--;
        sm = submaps[smx][smy][smz];
      }
      if (sm) {
        debugmsg("Really could not find a glyph! %s",
                 sm->get_spec_name().c_str());
      } else {
        debugmsg("Really could not find a glyph - invalid submap!");
      }
    }
    return;
  }
  if (invert) {
    output = output.invert();
  }
  if (gray) {
    output.fg = c_dkgray;
  }
  w->putglyph(tile_winx, tile_winy, output);
}
  

Submap* Map::get_center_submap()
{
  return submaps[MAP_SIZE / 2][MAP_SIZE / 2][VERTICAL_MAP_SIZE];
}

Submap* Map::get_testing_submap()
{
  return submaps[MAP_SIZE / 2][MAP_SIZE / 2 - 1][VERTICAL_MAP_SIZE];
}


Point Map::get_center_point()
{
  return Point(posx + MAP_SIZE / 2, posy + MAP_SIZE / 2);
}

// TODO: Clean this up?
Tripoint Map::find_item(Item* it, int uid)
{
// Sanity check
  if (it == NULL && uid < 0) {
    return Tripoint(-1, -1, -1);
  }
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      for (int z = 0; z < VERTICAL_MAP_SIZE * 2 + 1; z++) {
        Submap* sm = submaps[x][y][z];
        int rz = z - VERTICAL_MAP_SIZE + posz;
        if (sm) {
          for (int sx = 0; sx < SUBMAP_SIZE; sx++) {
            for (int sy = 0; sy < SUBMAP_SIZE; sy++) {
              int rx = x * SUBMAP_SIZE + sx;
              int ry = y * SUBMAP_SIZE + sy;
              std::vector<Item>* items = sm->items_at(sx, sy);
              if (!items) {
                debugmsg("NULL Items in Map::find_item_uid()");
              }
              for (int i = 0; i < items->size(); i++) {
                if ( &( (*items)[i] ) == it || (*items)[i].get_uid() == uid ) {
                  return Tripoint(rx, ry, rz);
                }
              }
            }
          }
        }
      }
    }
  }
// If we never found it... return nothing point
  return Tripoint(-1, -1, -1);
}

Tripoint Map::find_item_uid(int uid)
{
  return find_item(NULL, uid);
}
