#ifndef _ITEM_TYPE_H_
#define _ITEM_TYPE_H_

#include "glyph.h"
#include "enum.h"
#include "dice.h"
#include "tool.h"
#include <string>
#include <istream>
#include <vector>

/* After adding an Item_class, be sure to edit Item_type.cpp and add the class
 * to the function item_class_name().
 */
enum Item_class
{
  ITEM_CLASS_MISC = 0,
  ITEM_CLASS_CLOTHING,
  ITEM_CLASS_AMMO,
  ITEM_CLASS_LAUNCHER,
  ITEM_CLASS_FOOD,
  ITEM_CLASS_TOOL,
  ITEM_CLASS_MAX
};

Item_class lookup_item_class(std::string name);
std::string item_class_name_plural(Item_class iclass);
std::string item_class_name(Item_class iclass, bool plural = false);

enum Item_flag
{
  ITEM_FLAG_NULL = 0,
  ITEM_FLAG_LIQUID,     // "liquid" - Puts out fires, needs a container
  ITEM_FLAG_FLAMMABLE,  // "flammable" - Consumed by fires
  ITEM_FLAG_MAX
};

Item_flag lookup_item_flag(std::string name);
std::string item_flag_name(Item_flag flag);

class Item_type
{
public:
  Item_type();
  virtual ~Item_type();

  int uid;
  std::string name;
  std::string display_name;
  std::string description;
  int weight;       // In 1/10ths of a pound / 0.045 kg (sorry)
  int volume;       // 1 volume = a ping pong ball
  glyph sym;

  int damage[DAMAGE_MAX];
  int to_hit;
  int attack_speed;
  Dice  thrown_variance;// Angle our thrown attack is off by, in 1/10ths of
                        // a degree; defaults to 5d20!
  int thrown_dmg_percent; // Percent of normal damage done when thrown
  int thrown_speed; // AP cost of throwing; if 0, it's calculated

  void assign_uid(int id);
  std::string get_data_name();
  std::string get_name();
  virtual bool load_data(std::istream &data);
  virtual bool handle_data(std::string ident, std::istream &data);

  virtual Item_class get_class() { return ITEM_CLASS_MISC; }
  virtual Item_action default_action() { return IACT_NULL; }
  virtual int  time_to_reload()  { return 0; }
  virtual int  time_to_fire()    { return 0; }
  virtual int  default_charges() { return 0; }
  virtual bool always_combines()    { return false; }
  virtual bool combine_by_charges() { return false; }
  bool has_flag(Item_flag flag);

private:
  std::vector<bool> flags;
};

class Item_type_clothing : public Item_type
{
public:
  Item_type_clothing();
  ~Item_type_clothing(){}

  virtual Item_class get_class() { return ITEM_CLASS_CLOTHING; }
  virtual Item_action default_action() { return IACT_WEAR; }

  virtual bool handle_data(std::string ident, std::istream &data);

  int carry_capacity;
  int armor_bash;
  int armor_cut;
  int armor_pierce;
  int encumbrance;
  bool covers[BODY_PART_MAX];

};

class Item_type_ammo : public Item_type
{
public:
  Item_type_ammo();
  ~Item_type_ammo(){}

  virtual Item_class get_class() { return ITEM_CLASS_AMMO; }

  virtual bool handle_data(std::string ident, std::istream &data);

  virtual int default_charges() { return count; }
  virtual bool always_combines()    { return true;  }
  virtual bool combine_by_charges() { return true;  }

  std::string ammo_type;  // Ammo type - links this to a launcher
  int damage;       // Base damage
  int armor_pierce; // Armor ignored
  int range;
  Dice accuracy;     // Low is good!  In 1/10ths of a degree
  int count;        // How many to a box
};

class Item_type_launcher : public Item_type
{
public:
  Item_type_launcher();
  ~Item_type_launcher(){}

  virtual Item_class get_class() { return ITEM_CLASS_LAUNCHER; }

  virtual bool handle_data(std::string ident, std::istream &data);

  virtual int time_to_reload() { return reload_ap; }
  virtual int time_to_fire()   { return fire_ap; }

  std::string ammo_type;  // Ammo type - links this to a launcher
  int damage;     // Damage bonus
  Dice accuracy;   // Low is good!  In 1/10ths of a degree
  int recoil;     // Recoil added
  int durability; // HP basically
  int capacity;   // Shots per reload
  int reload_ap;  // action_points per reload
  int fire_ap;    // action_points per shot fired
  std::vector<int> modes; // Each element is a number of shots

};

class Item_type_food : public Item_type
{
public:
  Item_type_food();
  ~Item_type_food(){};

  virtual Item_class get_class() { return ITEM_CLASS_FOOD; }
  virtual Item_action default_action() { return IACT_EAT; }

  virtual bool handle_data(std::string ident, std::istream &data);

  int food;
  int water;

};

class Item_type_tool : public Item_type
{
public:
  Item_type_tool();
  ~Item_type_tool(){};

  virtual Item_class get_class() { return ITEM_CLASS_TOOL; }
  virtual Item_action default_action() { return IACT_APPLY; }

  virtual bool handle_data(std::string ident, std::istream &data);

  bool uses_charges();  // true if max_charges > 0 && charges_per_use > 0

  Tool_action action; // see tool.h and tool.cpp
  Tool_target target; // ditto

  int action_ap;  // AP to use the action
  int default_charges;  // Charges it starts with
  int max_charges;      // Max charges.  If 0, doesn't use charges.
  int charges_per_use;  // Charges per use - defaults to 1
  std::string fuel;     // Ammo name - matches this with an ammo type for fuel
};

#endif
