#ifndef _DATAPOOL_H_
#define _DATAPOOL_H_

#include <string>
#include <istream>
#include <list>
#include <map>
#include <fstream>
#include "window.h"
#include "item_type.h"
#include "stringfunc.h"

/* Important notes:
 * All classes to be used with Data_pool must have the following functions:
 *   void assign_uid(int)           - Assign its UID (it's not required to
 *                                    actually STORE this, but the function must
 *                                    exist nevertheless).
 *   bool load_data(std::istream&)  - Load data from a stream. Should return
 *                                    false on failure, true on success.
 *   std::string get_name()         - Return its name
 */

// Everything is inline cause compilers are dumb etc. etc.

template <class T>
struct Data_pool
{
public:
  Data_pool()
  {
    next_uid = 0;
  };

  virtual ~Data_pool()
  {
    for (typename std::list<T*>::iterator it = instances.begin();
         it != instances.end();
         it++) {
      if (*it) {
        delete (*it);
      }
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
    name_map[ no_caps(tmp->get_name()) ] = tmp;
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
    name = no_caps(name);
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
