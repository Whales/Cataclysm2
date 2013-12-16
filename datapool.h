#ifndef _DATAPOOL_H_
#define _DATAPOOL_H_

#include <string>
#include <istream>
#include <list>
#include <map>
#include <fstream>
#include "window.h"

/* Important notes:
 * All classes to be used with Data_pool must have the following functions:
 *   void assign_uid(int)           - Assign its UID
 *   bool load_data(std::istream&)  - Load data from a stream
 *     Should return false on failure, true on success.
 *   std::string get_name()         - Return its name
 */

template <class T>
struct Data_pool
{
public:
  Data_pool()
  {
    next_uid = 0;
  };

  ~Data_pool()
  {
    for (typename std::list<T*>::iterator it = instances.begin();
         it != instances.end();
         it++) {
      delete (*it);
    }
  };

  bool load_from(std::string filename)
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
  };

  bool load_element(std::istream &data)
  {
    T* tmp = new T;
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

  T* lookup_uid(int uid)
  {
    if (uid_map.count(uid) == 0) {
      return NULL;
    }
  
    return uid_map[uid];
  }

  T* lookup_name(std::string name)
  {
    if (name_map.count(name) == 0) {
      return NULL;
    }
  
    return name_map[name];
  }

  int size()
  {
    return instances.size();
  }

  std::list<T*> instances;

private:
  std::map<int,T*> uid_map;
  std::map<std::string,T*> name_map;
  int next_uid;
};

#endif
