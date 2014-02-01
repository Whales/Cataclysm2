#ifndef _FIELD_H_
#define _FIELD_H_

#include "enum.h" // For Body_part
#include "attack.h" // For Damage_set
#include "glyph.h"
#include <string>
#include <vector>
#include <list>
#include <istream>

/* If you're familiar with them, fields in Cataclysm 2 are very similar to those
 * in Cataclysm 1.
 * TODO: Do we need a "dangerous" flag to warn players against stepping into a
 *       field?
 */
class Field_level
{
public:
  Field_level();
  ~Field_level();

// TODO: Do we need a display_name too?
  std::string name;
  glyph sym;

  int duration; // Our starting "hp," lose one per turn

  std::string verb; // The fire [burns] you!  The electricity [shocks] you!
  Damage_set damage;  
  std::list<Body_part> body_parts_hit;

// TODO:  Add status effect inflicted
//        And anything else?

  bool load_data(std::istream& data, std::string owner_name);

  std::string get_name();
  bool has_flag(Terrain_flag tf);

private:
/* terrain_flags is mostly included so we can make a field block LoS, but it
 * could have other uses too...?
 */
  std::vector<bool> terrain_flags; // Same as terrain uses!

};

class Field_type
{
public:
  Field_type();
  ~Field_type();

  std::string name;
  std::string display_name;
  int uid;

  std::string get_data_name();
  std::string get_name();
  std::string get_level_name(int level);
  Field_level* get_level(int level);

  int get_uid();

  bool load_data(std::istream& data);

private:
  std::vector<Field_level*> levels;
};

#endif
