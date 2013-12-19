#ifndef _ITEMTYPE_H_
#define _ITEMTYPE_H_

#include <string>
#include <istream>
#include "glyph.h"

class Itemtype
{
public:
  Itemtype();
  virtual ~Itemtype();

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

class Itemtype_clothing : public Itemtype
{
public:
  Itemtype_clothing(){};
  ~Itemtype_clothing(){};
};

#endif
