#ifndef _item_type_H_
#define _item_type_H_

#include "glyph.h"
#include "enum.h"
#include <string>
#include <istream>

enum Item_class
{
  ITEM_CLASS_MISC = 0,
  ITEM_CLASS_CLOTHING,
  ITEM_CLASS_MAX
};

Item_class lookup_item_class(std::string name);
std::string item_class_name(Item_class iclass);

class Item_type
{
public:
  Item_type();
  virtual ~Item_type();

  int uid;
  std::string name;
  std::string description;
  int weight;       // In 1/10ths of a pound
  int volume;       // 1 volume = a ping pong ball
  glyph sym;

  int bash;
  int cut;
  int pierce;
  int damage[DAMAGE_MAX];
  int to_hit;
  int attack_speed;

  void assign_uid(int id);
  std::string get_name();
  virtual bool load_data(std::istream &data);
  virtual bool handle_data(std::string ident, std::istream &data);

  virtual Item_class get_class() { return ITEM_CLASS_MISC; };

private:
};

class Item_type_clothing : public Item_type
{
public:
  Item_type_clothing();
  ~Item_type_clothing(){};

  virtual Item_class get_class() { return ITEM_CLASS_CLOTHING; };

  virtual bool handle_data(std::string ident, std::istream &data);

  int carry_capacity;
  int armor_bash;
  int armor_cut;
  int armor_pierce;
  int encumbrance;
  bool covers[BODYPART_MAX];

};

#endif
