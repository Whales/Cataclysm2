#include "datapool.h"

template <>
Data_pool<Itemtype>::~Data_pool()
{
  std::list<Itemtype*>::iterator it = instances.begin();
  while (it != instances.end()) {
    delete (*it);
    it = instances.erase(it);
  }
}

template <>
bool Data_pool<Itemtype>::load_element(std::istream &data)
{
  Itemtype* tmp;
  std::string item_category;
  std::getline(data, item_category);
  item_category = no_caps(item_category);
  item_category = trim(item_category);
  if (item_category == "weapon" || item_category == "vanilla") {
    tmp = new Itemtype;
  } else if (item_category == "armor" || item_category == "armour" ||
             item_category == "clothing") {
    tmp = new Itemtype_clothing;
  }
  if (!tmp->load_data(data)) {
    return false;
  }
  tmp->assign_uid(next_uid);
  instances.push_back(tmp);
  uid_map[next_uid] = tmp;
  name_map[tmp->get_name()] = tmp;
  next_uid++;
  return true;
};
