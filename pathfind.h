#ifndef _PATHFIND_H_
#define _PATHFIND_H_

#include "geometry.h"
#include <vector>

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

  std::vector<Point> get_points();
  int get_cost();

  Point step(int n);
  Point operator[](int n);
  int size() { return path.size(); }
  

  void add_step(Point p, int cost);
  void reverse();
private:
  std::vector<Point> path;
  int total_cost;
};

class Generic_map
{
public:
  Generic_map(int x = 0, int y = 0);
  ~Generic_map();

  void set_size(int x, int y);
  void set_cost(int x, int y, int c);

  int  get_size_x();
  int  get_size_y();
  int  get_cost(int x, int y);
  int  get_cost(Point p);
  bool blocked(int x, int y);
  bool blocked(Point p);
private:
  std::vector<std::vector<int> > cost;
};

class Pathfinder
{
public:
  Pathfinder();
  Pathfinder(Generic_map m);
  ~Pathfinder();

  void set_map(Generic_map m);

  void set_bounds(int x0, int y0, int x1, int y1);
  void set_bounds(Point p0, Point p1);
  void set_bounds(int b);

  void set_allow_diagonal(bool allow = true);

  bool in_bounds(int x, int y);
  bool in_bounds(Point p);
  
  Path get_path(Path_type type, int x0, int y0, int x1, int y1);
  Path get_path(Path_type type, Point start, Point end);

private:
  Generic_map map;
  int x_min, x_max, y_min, y_max;
  int border;
  bool allow_diag;

  Path path_line(Point start, Point end);
  Path path_a_star(Point start, Point end);
};

#endif
