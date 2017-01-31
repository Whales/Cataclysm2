#ifndef _MAP_H_
#define _MAP_H_

#define SUBMAP_SIZE 25
#define MAP_SIZE 13
#define VERTICAL_MAP_SIZE 3

#include "window.h"
#include "terrain.h"
#include "world_terrain.h"
#include "mapgen.h"
#include "worldmap.h"
#include "item.h"
#include "enum.h"
#include "geometry.h"
#include "attack.h"
#include "pathfind.h"
#include "field.h"
#include <istream>

class Entity_pool;
class Entity;

struct Furniture
{
  Furniture();
  ~Furniture();

  void set_type(Furniture_type* t);
  void set_uid(int id);

// Data access
  bool is_real();
  int get_uid();
  glyph get_glyph();
  int move_cost();
  int get_height();
  int get_weight();
  std::string get_name();
  bool has_flag(Terrain_flag flag);

// Modifiers
  bool is_smashable();
  std::string smash(Damage_set dam);  // Returns the sound
// These return true if the furniture is destroyed.
  bool damage(Damage_set dam);
  bool damage(Damage_type damtype, int dam);
  void destroy(); // Reset type to NULL

  std::string save_data();
  bool load_data(std::istream& data);

// Properties
  Furniture_type *type;
  int hp;
  int uid;
};

// This is a Furniture, along with its RELATIONAL position - e.g. when dragged
// So if (pos) is [1, 1] then the piece of furniture is one tile south-east
struct Furniture_pos
{
  Furniture furniture;
  Point pos;
};

struct Sight_map
{
public:
  Sight_map();
  ~Sight_map();

  void make_initialized();
// Adds the point to our vector, inserting it in the correct location.
  void add_point(Tripoint p);

  bool is_initialized() const;
  bool can_see(Tripoint p) const;

private:
  bool initialized;
  std::vector<Tripoint> seen;
};

struct Tile
{
  Terrain *terrain;
  Sight_map sight_map;

  std::vector<Item> items;
  Field field;
  Furniture furniture;
  int hp;

   Tile() { hp = 0; }
  ~Tile() { }

  void set_terrain(Terrain* ter);
  void add_furniture(Furniture_type* type, int UID = -1);
  void add_furniture(Furniture furn);
  void remove_furniture();

// Basic data fetching - typically draws from Terrain, but might use Furniture
  glyph top_glyph();
  int move_cost();
  int get_height();
  std::string get_name();
  std::string get_name_indefinite();  // With indefinite article
  bool blocks_sense(Sense_type sense = SENSE_SIGHT, int z_value = 50);
  bool has_flag(Terrain_flag flag);

  bool has_field();
  bool has_furniture();

  bool is_smashable();
  std::string smash(Damage_set dam);  // Returns the sound
  bool damage(Damage_set dam);            // Returns true on destruction
  bool damage(Damage_type type, int dam); // Returns true on destruction
  void destroy(); // Happens if hp <= 0
  bool signal_applies(std::string signal);
  bool apply_signal  (std::string signal, Entity* user = NULL);

  std::string save_data();
  bool load_data(std::istream& data);
};

struct Submap
{
  Tile tiles[SUBMAP_SIZE][SUBMAP_SIZE];

/* spec_used is the Mapgen_spec used to generate this.  It's tracked strictly
 * for debugging purposes.
 * Subname is the specific flavor of the mapgen_spec used here.  This is used
 * when building second stories; so only use house_wood to build the 2nd floor
 * of a house_wood map.
 * rotation and level are used to further match to the floor below.
 */
  Mapgen_spec* spec_used;
  std::string subname;
  Direction rotation;
  int level;

  Submap();
  ~Submap();

  void generate_empty();
  void generate_open();

  void generate(Worldmap* map, int posx, int posy, int posz = 0);
  void generate(World_terrain* terrain[5], int posz = 0);
  //void generate(std::string terrain_name);
  void generate(Mapgen_spec* spec);
  void generate_adjacent(Mapgen_spec* spec);
  void generate_above(World_terrain* type, Submap* below);

  void clear_items();
  bool add_item(Item item, int x, int y);
  int  item_count(int x, int y);
  std::vector<Item>* items_at(int x, int y);

  Point random_empty_tile();

  std::string get_spec_name();
  std::string get_world_ter_name();

  std::string save_data();
  bool load_data(std::istream& data);

};

/* So: We need to limit how many submaps are in memory at any given time,
 * because they use a lot of memory and we'll quickly run out.
 * The idea here is to divide the world into "sectors," and have nine sectors -
 * a 3x3 grid - in memory at once.  Once the player moves out of the center
 * sector, we move/reload sectors so that the player is repositioned in the
 * center.
 */

#define SECTOR_SIZE 15

struct Submap_pool
{
public:
  Submap_pool();
  ~Submap_pool();
  Submap* at_location(int x, int y, int z = 0);
  Submap* at_location(Point p);
  Submap* at_location(Tripoint p);

/* load_area() loads nine sectors, with the upper-left corner at
 * [SECTOR_SIZE * sector_x][SECTOR_SIZE * sector_y].  It saves all current data,
 * runs through (instances) o find any submaps which aren't in that area, and
 * removes them; it then loads or generates any missing submaps.
 */
  void load_area(int sector_x, int sector_y);
/* load_area_centered_on() calls load_area(), passing a sector_x and sectory_y
 * which will place [center_x][center_y] in the center sector.
 */
  void load_area_centered_on(int center_x, int center_y);

  int size();
  std::string all_size();

// For debugging purposes.
  std::string get_range_text();

  std::list<Submap*> instances;
  Point sector;

private:
  std::map<Tripoint,Submap*,Tripointcomp> point_map;

  void remove_point(Tripoint p);
  void remove_submap(Submap* sm);

  void clear_submaps(int sector_x, int sector_y);
  void init_submaps (int sector_x, int sector_y);
  bool load_submaps(std::string filename);

  Submap* generate_submap(int x, int y, int z = 0);
  Submap* generate_submap(Tripoint p);

};

class Map
{
public:
  Map();
  ~Map();

// Generation
  void generate_empty();
  //void test_generate(std::string terrain_name);
  void generate(Worldmap *world, int wposx = -999, int wposy = -999,
                                 int wposz = -999);
  void shift(Worldmap *world, int shiftx, int shifty, int shiftz = 0);
  void spawn_monsters(Worldmap *world, int worldx, int worldy,
                      int subx, int suby, int zlevel);

// Mapping, pathing, LoS
  Generic_map get_dijkstra_map(Tripoint target, int weight,
                               bool include_smashable = true);
  Generic_map get_movement_map(Entity_AI AI, Tripoint origin, Tripoint target);

// If force_rebuild is false, we skip any tiles for which the Sight_map is
// already initialized.
  void build_sight_map(int range = -1, bool force_rebuild = false);
// FIXME: helper, shouldn't de declared here?
  void build_tile_sight_map(int tile_x, int tile_y, int tile_z, int range);

  bool senses(int x0, int y0, int x1, int y1, int range, Sense_type sense);
  bool senses(int x0, int y0, int z0, int x1, int y1, int z1, int range,
              Sense_type sense);
  bool senses(Point origin, Point target, int range, Sense_type sense);
  bool senses(Tripoint origin, Tripoint target, int range, Sense_type sense);

// clear_path() uses line_of_sight() to detemine if there's an movement path
  bool clear_path_exists(Tripoint origin, Tripoint target, int range = -1);
  bool clear_path_exists(int x0, int y0, int z0,
                         int x1, int y1, int z1, int range = -1);

  std::vector<Tripoint> clear_path(Tripoint origin, Tripoint target);
  std::vector<Tripoint> clear_path(int x0, int y0, int z0,
                                   int x1, int y1, int z1);

  std::vector<Tripoint> line_of_sight(int x0, int y0, int x1, int y1);
  std::vector<Tripoint> line_of_sight(int x0, int y0, int z0,
                                   int x1, int y1, int z1);
  std::vector<Tripoint> line_of_sight(Point origin, Point target);
  std::vector<Tripoint> line_of_sight(Tripoint origin, Tripoint target);

// Tile information
  int  move_cost(Tripoint pos);
  int  move_cost(int x, int y, int z = 999);

  int  get_height(Tripoint pos);
  int  get_height(int x, int y, int z = 999);

  bool is_smashable(Tripoint pos);
  bool is_smashable(int x, int y, int z = 999);

  bool has_flag(Terrain_flag flag, Tripoint pos);
  bool has_flag(Terrain_flag flag, int x, int y, int z = 999);

  bool blocks_sense(Sense_type sense, Tripoint pos, int z_value = 50);
  bool blocks_sense(Sense_type sense, int x, int y, int z = 999);

  bool add_item(Item item, Tripoint pos);
  bool add_item(Item item, int x, int y, int z = 999);

  void clear_items(); // Clears ALL items!
  bool remove_item(Item* it, int uid = -1);
  bool remove_item_uid(int uid);  // remove_item(NULL, uid);

  bool add_field(Field_type* type, Tripoint pos, std::string creator = "");
  bool add_field(Field_type* type, int x, int y, int z = 999,
                 std::string creator = "");
  bool add_field(Field field,      Tripoint pos);
  bool add_field(Field field,      int x, int y, int z = 999);

  int  item_count(Tripoint pos);
  int  item_count(int x, int y, int z = 999);

  std::vector<Item>* items_at(Tripoint pos);
  std::vector<Item>* items_at(int x, int y, int z = 999);

  Furniture* furniture_at(Tripoint pos);
  Furniture* furniture_at(int x, int y, int z = 999);
  void add_furniture(Furniture furn, Tripoint pos);
  void add_furniture(Furniture furn, int x, int y, int z = 999);
  std::vector<Furniture_pos> grab_furniture(Tripoint origin, Tripoint target,
                                            int id = -1,
                                         std::vector<Tripoint>* checked = NULL);
  void clear_furniture(Tripoint pos);
  void clear_furniture(int x, int y, int z = 999);

  bool contains_field(Tripoint pos);
  bool contains_field(int x, int y, int z = 999);
  Field* field_at(Tripoint pos);
  Field* field_at(int x, int y, int z = 999);
  int field_uid_at(Tripoint pos);
  int field_uid_at(int x, int y, int z = 999);

  Tile* get_tile(Tripoint pos);
  Tile* get_tile(int x, int y, int z = 999);

  std::string get_name(Tripoint pos);
  std::string get_name(int x, int y, int z = 999);
  std::string get_name_indefinite(Tripoint pos);
  std::string get_name_indefinite(int x, int y, int z = 999);

// Map-altering
  void smash(int x, int y,          Damage_set dam, bool make_sound = true);
  void smash(int x, int y, int z,   Damage_set dam, bool make_sound = true);
  void smash(Tripoint pos,          Damage_set dam, bool make_sound = true);
  void damage(int x, int y,         Damage_set dam);
  void damage(int x, int y, int z,  Damage_set dam);
  void damage(Tripoint pos,         Damage_set dam);

  bool apply_signal(std::string signal, Tripoint pos, Entity* user = NULL);
  bool apply_signal(std::string signal, int x, int y, int z,
                    Entity* user = NULL);

// Regularly-run functions
  void process_fields();

// Output
  void draw(Window *w, Entity_pool *entities, Tripoint ref,
            int range = -1, Sense_type sense = SENSE_SIGHT);
  void draw(Window *w, Entity_pool *entities, int refx, int refy, int refz,
            int range = -1, Sense_type sense = SENSE_SIGHT);
  void draw_area(Window *w, Entity_pool *entities, Tripoint ref,
                 int minx, int miny, int maxx, int maxy,
                 int range = -1, Sense_type sense = SENSE_SIGHT);
  void draw_area(Window *w, Entity_pool *entities, int refx, int refy, int refz,
                 int minx, int miny, int maxx, int maxy,
                 int range = -1, Sense_type sense = SENSE_SIGHT);

  void draw_tile(Window* w, Entity_pool *entities, int tilex, int tiley,
                 int refx, int refy, bool invert, bool gray = false);
  void draw_tile(Window* w, Entity_pool *entities,
                 int tilex, int tiley, int tilez,
                 int refx, int refy, bool invert, bool gray = false);

// Other information
  Submap* get_center_submap(); // i.e. the one the player is in
  Submap* get_testing_submap(); // Just north of us
  Point get_center_point();
  Tripoint find_item(Item* it, int uid = -1);
  Tripoint find_item_uid(int uid); // find_item(NULL, uid)

  std::string get_range_text(); // For debugging purposes.

  int posx, posy, posz; // Worldmap coordinates of the upper-left submap.

private:
  Submap* submaps[MAP_SIZE][MAP_SIZE][VERTICAL_MAP_SIZE * 2 + 1];
  Tile tile_oob;
  std::vector<Tripoint> field_points;
};
#endif
