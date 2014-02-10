#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "glyph.h"
#include "enum.h"
#include "tool.h"
#include <vector>
#include <map>

Terrain_flag lookup_terrain_flag(std::string name);
std::string terrain_flag_name(Terrain_flag flag);

struct Terrain;

struct Terrain_smash
{
  Terrain_smash();
  ~Terrain_smash(){}

  std::string failure_sound;
  std::string success_sound;
  int armor[DAMAGE_MAX];
  int ignore_chance;

  bool load_data(std::istream &data, std::string name = "unknown");
};

struct Terrain
{
  int uid;
  std::string name;
  std::string display_name;
  glyph sym;
  unsigned int movecost;
/* Height ranged from 0-100 and reflects how much vertical space is blocked (for
 * line of sight calculations).  It defaults to 100.
 */
  unsigned int height;
  unsigned int hp;      // Defaults to 0.  0 HP means it's indestructible.

  Terrain_smash smash;
  bool smashable;
  std::string open_result;
  std::string close_result;
  std::string destroy_result;
// A map of what happens when a Tool_action is applied
  std::map<Tool_action,std::string> tool_result;

  bool can_smash() { return smashable; }
  bool can_open()  { return !open_result.empty();  }
  bool can_close() { return !close_result.empty(); }

  Terrain();
  ~Terrain(){}

  std::string get_data_name();
  std::string get_name();
  void assign_uid(int id) { uid = id; }
  bool load_data(std::istream &data);
  bool has_flag(Terrain_flag flag);
private:
  std::vector<bool> flags;
};

#endif
