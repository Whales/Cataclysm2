#include <fstream>
#include "window.h"
#include "mapgen.h"
#include "rng.h"

// First, the mapgen_spec_pool that we'll use to store mapgen data.

Mapgen_spec_pool::Mapgen_spec_pool()
{
  next_uid = 0;
}

Mapgen_spec_pool::~Mapgen_spec_pool()
{
  for (std::list<Mapgen_spec*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    delete (*it);
  }
}

bool Mapgen_spec_pool::load_from(std::string filename)
{
  std::ifstream fin;
  fin.open(filename.c_str());
  if (!fin.is_open()) {
    debugmsg("Failed to open '%s'", filename.c_str());
    return false;
  }

  while (!fin.eof()) {
    if (!load_element(fin)) {
      return false;
    }
  }
  return true;
}

bool Mapgen_spec_pool::load_element(std::istream &data)
{
  Mapgen_spec* tmp = new Mapgen_spec;
  if (!tmp->load_data(data)) {
    return false;
  }
  tmp->uid = next_uid;
  instances.push_back(tmp);
  uid_map[next_uid] = tmp;
  name_map[tmp->name] = tmp;
  if (terrain_name_map.count(tmp->terrain_name) == 0) {
    std::vector<Mapgen_spec*> tmpvec;
    tmpvec.push_back(tmp);
    terrain_name_map[tmp->terrain_name] = tmpvec;
  } else {
    terrain_name_map[tmp->terrain_name].push_back(tmp);
  }
  return true;
}

Mapgen_spec* Mapgen_spec_pool::lookup_uid(int uid)
{
  if (uid_map.count(uid) == 0) {
    return NULL;
  }
  return uid_map[uid];
}

Mapgen_spec* Mapgen_spec_pool::lookup_name(std::string name)
{
  if (name_map.count(name) == 0) {
    return NULL;
  }
  return name_map[name];
}

std::vector<Mapgen_spec*>
Mapgen_spec_pool::lookup_terrain_name(std::string name)
{
  if (terrain_name_map.count(name) == 0) {
    std::vector<Mapgen_spec*> tmp;
    return tmp;
  }
  return terrain_name_map[name];
}

Mapgen_spec* Mapgen_spec_pool::random_for_terrain(std::string name)
{
  if (terrain_name_map.count(name) == 0) {
    return NULL;
  }
  int index = rng(0, terrain_name_map[name].size() - 1);
  return terrain_name_map[name][index];
}

int Mapgen_spec_pool::size()
{
  return instances.size();
}
