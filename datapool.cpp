#include "datapool.h"

template <>
Data_pool<Item_type>::~Data_pool()
{
  std::list<Item_type*>::iterator it = instances.begin();
  while (it != instances.end() && instances.size() > 1) {
    if (*it) {
      delete (*it);
    }
    it = instances.erase(it);
  }
}

template <>
bool Data_pool<Item_type>::load_element(std::istream &data)
{
  Item_type* tmp;
  std::string item_category;
  //std::getline(data, item_category);
  data >> item_category;
  item_category = no_caps(item_category);
  item_category = trim(item_category);
  if (item_category == "weapon" || item_category == "vanilla") {
    tmp = new Item_type;
  } else if (item_category == "armor" || item_category == "armour" ||
             item_category == "clothing") {
    tmp = new Item_type_clothing;
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
