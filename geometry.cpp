#include "geometry.h"
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <sstream> // For Tripoint::string()

#define SGN(a) (((a)<0) ? -1 : 1)

std::vector <Point> line_to(int x0, int y0, int x1, int y1)
{
 int t = 0;
 std::vector<Point> ret;
 int dx = x1 - x0;
 int dy = y1 - y0;
 int ax = abs(dx)<<1;
 int ay = abs(dy)<<1;
 int sx = SGN(dx);
 int sy = SGN(dy);
 if (dy == 0) sy = 0;
 if (dx == 0) sx = 0;
 Point cur;
 cur.x = x0;
 cur.y = y0;

 int xmin = (x0 < x1 ? x0 : x1), ymin = (y0 < y1 ? y0 : y1),
     xmax = (x0 > x1 ? x0 : x1), ymax = (y0 > y1 ? y0 : y1);

 xmin -= abs(dx);
 ymin -= abs(dy);
 xmax += abs(dx);
 ymax += abs(dy);

 if (ax == ay) {
  do {
   cur.y += sy;
   cur.x += sx;
   ret.push_back(cur);
  } while ((cur.x != x1 || cur.y != y1) &&
           (cur.x >= xmin && cur.x <= xmax && cur.y >= ymin && cur.y <= ymax));
 } else if (ax > ay) {
  do {
   if (t > 0) {
    cur.y += sy;
    t -= ax;
   }
   cur.x += sx;
   t += ay;
   ret.push_back(cur);
  } while ((cur.x != x1 || cur.y != y1) &&
           (cur.x >= xmin && cur.x <= xmax && cur.y >= ymin && cur.y <= ymax));
 } else {
  do {
   if (t > 0) {
    cur.x += sx;
    t -= ay;
   }
   cur.y += sy;
   t += ax;
   ret.push_back(cur);
  } while ((cur.x != x1 || cur.y != y1) &&
           (cur.x >= xmin && cur.x <= xmax && cur.y >= ymin && cur.y <= ymax));
 }
 return ret;
}

std::vector<Point> line_to(Point origin, Point target)
{
  return line_to(origin.x, origin.y, target.x, target.y);
}

int rl_dist(int x0, int y0, int x1, int y1)
{
  int dx = (x0 > x1 ? x0 - x1 : x1 - x0), dy = (y0 > y1 ? y0 - y1 : y1 - y0);
  return (dx > dy ? dx : dy);
}

int rl_dist(Point origin, Point target)
{
  return rl_dist(origin.x, origin.y, target.x, target.y);
}

int rl_dist(int x0, int y0, int z0, int x1, int y1, int z1)
{
  int dx = (x0 > x1 ? x0 - x1 : x1 - x0), dy = (y0 > y1 ? y0 - y1 : y1 - y0),
      dz = (z0 > z1 ? z0 - z1 : z1 - z0);
  if (dx > dy && dx > dz) {
    return dx;
  }
  if (dy > dz) {
    return dy;
  }
  return dz;
}

std::string Point::str()
{
  std::stringstream ret;
  ret << "[" << x << ":" << y << "]";
  return ret.str();
}

std::string Tripoint::str()
{
  std::stringstream ret;
  ret << "[" << x << ":" << y << ":" << z << "]";
  return ret.str();
}

int rl_dist(Tripoint origin, Tripoint target)
{
  return rl_dist(origin.x, origin.y, origin.z, target.x, target.y, target.z);
}

int manhattan_dist(int x0, int y0, int x1, int y1)
{
  return abs(x1 - x0) + abs(y1 - y0);
}

int manhattan_dist(Point origin, Point target)
{
  return manhattan_dist(origin.x, origin.y, target.x, target.y);
}

int manhattan_dist(int x0, int y0, int z0, int x1, int y1, int z1)
{
  return abs(x1 - x0) + abs(y1 - y0) + abs(z1 - z0);
}

int manhattan_dist(Tripoint origin, Tripoint target)
{
  return manhattan_dist(origin.x, origin.y, origin.z,
                        target.x, target.y, target.z);
}

Direction_full get_general_direction(Point origin, Point target)
{
  int dx = target.x - origin.x;
  int dy = target.y - origin.y;
  int ax = abs(dx), ay = abs(dy);

  if (dx == 0) {
    if (dy == 0) {
      return DIRFULL_NULL;
    } else if (dy > 0) {
      return DIRFULL_SOUTH;
    } else {
      return DIRFULL_NORTH;
    }
  } else if (dx > 0) {
    if (dy >= 0) {
      if (ay * 2 >= ax) {
        return DIRFULL_SOUTH;
      } else if (ax * 2 >= ay) {
        return DIRFULL_EAST;
      } else {
        return DIRFULL_SOUTHEAST;
      }
    } else {
      if (ay * 2 > ax) {
        return DIRFULL_NORTH;
      } else if (ax * 2 >= ay) {
        return DIRFULL_EAST;
      } else {
        return DIRFULL_NORTHEAST;
      }
    }
  } else {
    if (dy >= 0) {
      if (ay * 2 >= ax) {
        return DIRFULL_SOUTH;
      } else if (ax * 2 >= ay) {
        return DIRFULL_WEST;
      } else {
        return DIRFULL_SOUTHWEST;
      }
    } else {
      if (ay * 2 > ax) {
        return DIRFULL_NORTH;
      } else if (ax * 2 >= ay) {
        return DIRFULL_WEST;
      } else {
        return DIRFULL_NORTHWEST;
      }
    }
  }
  return DIRFULL_NULL;
}

Direction_full get_general_direction(Tripoint origin, Point target)
{
  return get_general_direction( Point(origin.x, origin.y), target );
}

Direction_full get_general_direction(Point origin, Tripoint target)
{
  return get_general_direction( origin, Point(target.x, target.y) );
}

Direction_full get_general_direction(Tripoint origin, Tripoint target)
{
  return get_general_direction( Point(origin.x, origin.y),
                                Point(target.x, target.y) );
}

std::string Direction_name(Direction dir)
{
  switch (dir) {
    case DIR_NULL:  return "None";
    case DIR_NORTH: return "North";
    case DIR_EAST:  return "East";
    case DIR_SOUTH: return "South";
    case DIR_WEST:  return "West";
  }
  return "???";
}

std::string Direction_name(Direction_full dir)
{
  switch (dir) {
    case DIRFULL_NULL:      return "None";
    case DIRFULL_NORTH:     return "North";
    case DIRFULL_NORTHEAST: return "Northeast";
    case DIRFULL_EAST:      return "East";
    case DIRFULL_SOUTHEAST: return "Southeast";
    case DIRFULL_SOUTH:     return "South";
    case DIRFULL_SOUTHWEST: return "Southwest";
    case DIRFULL_WEST:      return "West";
    case DIRFULL_NORTHWEST: return "Northwest";
  }
  return "???";
}
