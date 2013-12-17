#ifndef _MAPGEN_H_
#define _MAPGEN_H_

#include <list>
#include <string>
#include <istream>
#include "mapgen_spec.h"

/* Mapgen_spec_pool looks very much like a datapool but has a few special
 * features, so sadly it must be its own class.
 */
class Mapgen_spec_pool
{
public:
  Mapgen_spec_pool();
  ~Mapgen_spec_pool();

  bool load_from(std::string filename);
  bool load_element(std::istream &data);
  
  Mapgen_spec* lookup_uid(int uid);
  Mapgen_spec* lookup_name(std::string name);
  std::vector<Mapgen_spec*> lookup_terrain_name(std::string name);

  int size();

  std::list<Mapgen_spec*> instances;
private:
  int next_uid;
  std::map<int,Mapgen_spec*> uid_map;
  std::map<std::string,Mapgen_spec*> name_map;
  std::map<std::string,std::vector<Mapgen_spec*> > terrain_name_map;
};
#endif
