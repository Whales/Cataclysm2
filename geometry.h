#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <vector>

#ifndef MIN
  #define MIN(x, y) ( (x) < (y) ? x : y)
#endif

#ifndef MOD
  #define MOD(a, n) ( (a) < 0 ? ((a) % (n) + (n)) : ((a) % (n)))
#endif

enum Direction
{
  DIR_NULL = 0,
  DIR_NORTH,
  DIR_EAST,
  DIR_SOUTH,
  DIR_WEST
};

struct Point
{
  int x;
  int y;
  Point(int X = 0, int Y = 0) : x (X), y (Y) {}
  Point(const Point &p) : x (p.x), y (p.y) {}
  ~Point(){}
};

struct Pointcomp
{
 bool operator() (const Point &lhs, const Point &rhs) const
 {
  if (lhs.x < rhs.x) return true;
  if (lhs.x > rhs.x) return false;
  if (lhs.y < rhs.y) return true;
  if (lhs.y > rhs.y) return false;
  return false;
 };
};

std::vector<Point> line_to(int x0, int y0, int x1, int y1);

int rl_dist(int x0, int y0, int x1, int y1);

#endif
