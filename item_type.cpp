#include "item_type.h"
#include "stringfunc.h"
#include "window.h"

Item_type::Item_type()
{
  uid = -1;
  name = "bug";
  weight = 0;
  volume = 0;
  sym = glyph();
  bash = 0;
  cut = 0;
  pierce = 0;
}

Item_type::~Item_type()
{
}

void Item_type::assign_uid(int id)
{
  uid = id;
}

std::string Item_type::get_name()
{
  return name;
}

bool Item_type::load_data(std::istream &data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident)) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);

    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);

    } else if (ident == "weight:") {
      data >> weight;
      std::getline(data, junk);

    } else if (ident == "volume:") {
      data >> volume;
      std::getline(data, junk);

    } else if (ident == "bash:") {
      data >> bash;
      std::getline(data, junk);

    } else if (ident == "cut:") {
      data >> cut;
      std::getline(data, junk);

    } else if (ident == "pierce:") {
      data >> pierce;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown item_type flag '%s' (%s)", ident.c_str(), name.c_str());
    }
  }
// TODO: Flag loading.
  return true;
}
