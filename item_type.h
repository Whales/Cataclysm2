#ifndef _item_type_H_
#define _item_type_H_

#include <string>
#include <istream>
#include "glyph.h"

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

private:
};

class Item_type_clothing : public Item_type
{
public:
  Item_type_clothing(){};
  ~Item_type_clothing(){};
};

#endif
