#ifndef _item_type_H_
#define _item_type_H_

#include <string>
#include <istream>
#include "glyph.h"

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
  int weight;       // In 1/10ths of a pound
  int volume;       // 1 volume = a ping pong ball
  glyph sym;

  int bash;
  int cut;
  int pierce;

  void assign_uid(int id);
  std::string get_name();
  virtual bool load_data(std::istream &data);

  virtual Item_class get_class() { return ITEM_CLASS_MISC; };

private:
};

class Item_type_clothing : public Item_type
{
public:
  Item_type_clothing() : Item_type();
  ~Item_type_clothing(){};

  virtual Item_class get_class() { return ITEM_CLASS_CLOTHING; };

  int carry_capacity;
  int armor_bash;
  int armor_cut;
  int armor_pierce;

};

#endif
