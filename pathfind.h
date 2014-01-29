#ifndef _PATHFIND_H_
#define _PATHFIND_H_

#include "geometry.h"
#include <vector>

class Map;

enum Path_type
{
  PATH_NULL = 0,
  PATH_LINE,
  PATH_A_STAR,
};

class Path
{
public:
  Path();
  ~Path();

  std::vector<Tripoint> get_points();
  int get_cost();

  Tripoint step(int n);
  Tripoint operator[](int n);
  int  size()  { return path.size(); }
  bool empty() { return size() == 0; }

  void add_step(Tripoint p, int cost);
  void erase_step(int index);
  void reverse();
private:
  std::vector<Tripoint> path;
  int total_cost;
};

class Generic_map
{
public:
  Generic_map(int x = 0, int y = 0, int z = 1);
  ~Generic_map();

  void set_size(int x, int y, int z = 1);
  void set_cost(int x, int y, int c);
  void set_cost(int x, int y, int z, int c);
  void set_cost(Tripoint p, int c);

  int  get_size_x();
  int  get_size_y();
  int  get_size_z();

  int  get_cost(int x, int y, int z = 0);
  int  get_cost(Point p);
  int  get_cost(Tripoint p);

  bool blocked(int x, int y, int z = 0);
  bool blocked(Point p);
  bool blocked(Tripoint p);
private:
  std::vector<std::vector<std::vector<int> > > cost;
};

class Pathfinder
{
public:
  Pathfinder();
  Pathfinder(Generic_map m);
  ~Pathfinder();

  void set_map(Generic_map m);
  void get_map(Map* map);

  void set_bounds(int x0, int y0, int x1, int y1);
  void set_bounds(int x0, int y0, int z0, int x1, int y1, int z1);
  void set_bounds(Point p0, Point p1);
  void set_bounds(Tripoint p0, Tripoint p1);
  void set_bounds(int b); // As a "border" around the square formed by the
                          // start/end points

  void set_allow_diagonal(bool allow = true);

  bool in_bounds(int x, int y, int z = 0);
  bool in_bounds(Point p);
  bool in_bounds(Tripoint p);
  
  Path get_path(Path_type type, int x0, int y0, int x1, int y1);
  Path get_path(Path_type type, Point start, Point end);
  Path get_path(Path_type type, int x0, int y0, int z0, int x1, int y1, int z1);
  Path get_path(Path_type type, Tripoint start, Tripoint end);

  Tripoint get_step(Path_type type, int x0, int y0, int x1, int y1);
  Tripoint get_step(Path_type type, Point start, Point end);
  Tripoint get_step(Path_type type, int x0, int y0, int z0,
                                    int x1, int y1, int z1);
  Tripoint get_step(Path_type type, Tripoint start, Tripoint end);

  Generic_map map;

private:
  int x_min, x_max, y_min, y_max, z_min, z_max;
  int border;
  bool allow_diag;

  Path path_line  (Tripoint start, Tripoint end);
  Path path_a_star(Tripoint start, Tripoint end);
};

#endif
