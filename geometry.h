#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <vector>
#include <string>

#ifndef MIN
  #define MIN(x, y) ( (x) < (y) ? x : y)
#endif

#ifndef MOD
  #define MOD(a, n) ( (a) < 0 ? ((a) % (n) + (n)) : ((a) % (n)))
#endif

#ifndef PI
  #define PI 3.142
#endif

enum Direction
{
  DIR_NULL = 0,
  DIR_NORTH,
  DIR_EAST,
  DIR_SOUTH,
  DIR_WEST
};

enum Direction_full
{
  DIRFULL_NULL = 0,
  DIRFULL_NORTH,
  DIRFULL_NORTHEAST,
  DIRFULL_EAST,
  DIRFULL_SOUTHEAST,
  DIRFULL_SOUTH,
  DIRFULL_SOUTHWEST,
  DIRFULL_WEST,
  DIRFULL_NORTHWEST
};

std::string Direction_name(Direction dir);
std::string Direction_name(Direction_full dir);

struct Point
{
  int x;
  int y;
  Point(int X = 0, int Y = 0) : x (X), y (Y) {}
  Point(const Point &p) : x (p.x), y (p.y) {}
  ~Point(){}

  std::string str();

  bool operator==(const Point &other) const
  {
    return (x == other.x && y == other.y);
  }

  bool operator!=(const Point &other) const
  {
    return !(*this == other);
  }

  Point& operator +=(const Point &rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
};

inline Point operator+(Point lhs, const Point& rhs)
{
  lhs += rhs;
  return lhs;
}

struct Tripoint
{
  int x;
  int y;
  int z;
  Tripoint(int X = 0, int Y = 0, int Z = 0) : x (X), y (Y), z (Z) {}
  Tripoint(const Tripoint &p) : x (p.x), y (p.y), z (p.z) {}
  ~Tripoint(){}

  std::string str();

  bool operator==(const Tripoint &other) const
  {
    return (x == other.x && y == other.y && z == other.z);
  }

  bool operator!=(const Tripoint &other) const
  {
    return !(*this == other);
  }

  Tripoint& operator +=(const Tripoint &rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;

    return *this;
  }

  Tripoint& operator +=(const Point &rhs)
  {
    x += rhs.x;
    y += rhs.y;

    return *this;
  }

  operator Point()
  {
    Point ret;
    ret.x = x;
    ret.y = y;
    return ret;
  }
};

inline Tripoint operator+(Tripoint lhs, const Tripoint& rhs)
{
  lhs += rhs;
  return lhs;
}

inline Tripoint operator+(Tripoint lhs, const Point& rhs)
{
  lhs += rhs;
  return lhs;
}

struct Pointcomp
{
  bool operator() (const Point &lhs, const Point &rhs) const
  {
    if (lhs.x < rhs.x) return true;
    if (lhs.x > rhs.x) return false;
    if (lhs.y < rhs.y) return true;
    if (lhs.y > rhs.y) return false;
    return false;
  }
};

struct Tripointcomp
{
  bool operator() (const Tripoint &lhs, const Tripoint &rhs) const
  {
    if (lhs.x < rhs.x) return true;
    if (lhs.x > rhs.x) return false;
    if (lhs.y < rhs.y) return true;
    if (lhs.y > rhs.y) return false;
    if (lhs.z < rhs.z) return true;
    if (lhs.z > rhs.z) return false;
    return false;
  }
};

std::vector<Point> line_to(int x0, int y0, int x1, int y1);
std::vector<Point> line_to(Point origin, Point target);

int rl_dist       (int x0, int y0, int x1, int y1);
int rl_dist       (Point origin, Point target);
int rl_dist       (int x0, int y0, int z0, int x1, int y1, int z1);
int rl_dist       (Tripoint origin, Tripoint target);
int manhattan_dist(int x0, int y0, int x1, int y1);
int manhattan_dist(Point origin, Point target);
int manhattan_dist(int x0, int y0, int z0, int x1, int y1, int z1);
int manhattan_dist(Tripoint origin, Tripoint target);

/* Direction origin moves to reach target
 * This is GENERAL direction, which means that if target is 500 tiles to the
 * north and 5 tiles to the west, we return north since any reasonable
 * observe would call that the "north" and not the "northwest."
 * Generally, if dY >= 2 * dX then treat dX as 0, etc.
 * All four are functionally equivalent and ignore Z for now.
 */
Direction_full get_general_direction(Point origin,    Point target);
Direction_full get_general_direction(Tripoint origin, Point target);
Direction_full get_general_direction(Point origin,    Tripoint target);
Direction_full get_general_direction(Tripoint origin, Tripoint target);

#endif
