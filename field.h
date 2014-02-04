#ifndef _FIELD_H_
#define _FIELD_H_

#include "enum.h"       // For Body_part
#include "attack.h"     // For Damage_set
#include "enum.h"       // For Terrain_flag
#include "item_type.h"  // For Item_flag
#include "glyph.h"
#include "terrain.h"    // For Terrain*
#include "geometry.h"   // For Tripoint
#include <string>
#include <vector>
#include <list>
#include <istream>

class Entity;
class Map;

/* If you're familiar with them, fields in Cataclysm 2 are very similar to those
 * in Cataclysm 1.
 * TODO: Do we need a "dangerous" flag to warn players against stepping into a
 *       field?
 */

enum Field_flag
{
  FIELD_FLAG_NULL = 0,
  FIELD_FLAG_SOLID,     // "solid" - can be placed on solid terrain
  FIELD_FLAG_DIFFUSE,   // "diffuse" - spread even if the cost will destroy us
  FIELD_FLAG_MAX
};

Field_flag lookup_field_flag(std::string name);
std::string field_flag_name(Field_flag flag);

struct Field_fuel
{
  Field_fuel(Terrain_flag TF = TF_NULL, Item_flag IF = ITEM_FLAG_NULL,
             int M = 0, int D = 0) :
    terrain_flag (TF), item_flag(IF), fuel (M), damage (D) { }
  ~Field_fuel() { }

  Terrain_flag  terrain_flag;
  Item_flag     item_flag;
  int fuel;
// Damage done to the terrain each turn
// If 0, then the terrain is not damaged or destroyed at all.
  int damage;

  std::string output_field; // A field created as output, e.g. smoke
// Duration of the output; if a negative number is rolled, no output
  Dice output_duration;
                            

  bool load_data(std::istream& data, std::string owner_name = "Unknown");
};

class Field_level
{
public:
  Field_level();
  ~Field_level();

  std::string name;
  glyph sym;

  int duration; // Our starting "hp," lose one per turn
  int duration_lost_without_fuel; // Extra duration lost if there's no fuel

/* Danger defaults to 0.
 * If danger > 0, the player is warned before stepping in this field.
 * Added to the move_cost of the tile for field-aware monster pathing.
 */
  int danger;

  std::string verb; // The fire [burns] you!  The electricity [shocks] you!
  Damage_set damage;  
  std::list<Body_part> body_parts_hit;

/* TODO:  Add status effect inflicted
 *        And anything else?
 * TODO:  Maybe some flags for hard-coded effects; e.g. electricity stays close
 *        to walls
 */

  bool load_data(std::istream& data, std::string owner_name);

  std::string get_name();
  bool has_flag(Field_flag ff);
  bool has_flag(Terrain_flag tf);

private:
/* terrain_flags is mostly included so we can make a field block LoS, but it
 * could have other uses too...?
 */
  std::vector<bool> terrain_flags; // Same as terrain uses!
  std::vector<bool> field_flags;

};

class Field_type
{
public:
  Field_type();
  ~Field_type();

  std::string name;
  std::string display_name;
  int uid;

  std::list<Field_fuel> fuel;
  int spread_chance;  // Percentage chance each turn
  int spread_cost;    // Percentage of our duration lost when spreading
  int output_chance;  // Percentage chance of extra output each turn
  int output_cost;    // Percent of our duration lost when outputting
  std::string output_type;  // Name of the field we output

  void assign_uid(int id) { uid = id; }
  std::string  get_data_name();
  std::string  get_name();
  std::string  get_level_name(int level);
  Field_level* get_level(int level);

  int duration_needed_to_reach_level(int level);

  int get_uid();

  bool load_data(std::istream& data);

private:
  std::vector<Field_level*> levels;

};

// This one is actually used in Tile (part of Submaps)
class Field
{
public:
  Field(Field_type* T = NULL, int L = 0, std::string C = "");
  ~Field();

  Field_type* type;
  int level;
  int duration;
  bool dead;    // If true, this needs to be cleaned up
/* We use creator to tell the player what killed them; e.g. if creator is 
 * "a spitter zombie" then we got killed by "acid created by a spitter zombie"
 */
  std::string creator;  // Name of what created us

// Type information
  int get_type_uid() const;
  bool is_valid();
  bool has_flag(Field_flag flag);
  bool has_flag(Terrain_flag flag);
  std::string get_name();       // Type name
  std::string get_full_name();  // get_name() + " created by " + owner
  glyph top_glyph();

// Status information
  int get_full_duration() const;

// Active functions
  Field& operator+=(const Field& rhs);
  void hit_entity(Entity* entity);
  void process(Map* map, Tripoint pos);
  void gain_level();
  void lose_level();
  void adjust_level();  // Fixes level based on duration
private:
  bool consume_fuel(Map* map, Tripoint pos); // Returns true if we consume fuel
};
inline Field operator+(Field lhs, const Field& rhs)
{
  lhs += rhs;
  return lhs;
}

#endif
