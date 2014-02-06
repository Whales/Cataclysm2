#include "pathfind.h"
#include "rng.h"
#include "window.h"
#include <math.h>
#include <algorithm>

Path::Path()
{
  total_cost = 0;
}

Path::~Path()
{
}

std::vector<Tripoint> Path::get_points()
{
  return path;
}

int Path::get_cost()
{
  return total_cost;
}

Tripoint Path::step(int n)
{
  if (n < 0 || n >= path.size()) {
    return Tripoint(-1, -1, -1);
  }
  return path[n];
}

Tripoint Path::operator[](int n)
{
  return step(n);
}

void Path::add_step(Tripoint p, int cost)
{
  path.push_back(p);
  total_cost += cost;
}

void Path::erase_step(int index)
{
  if (index < 0 || index >= path.size()) {
    return;
  }
  path.erase(path.begin() + index);
}

void Path::reverse()
{
  std::reverse(path.begin(), path.end());
}

void Path::offset(int x_offset, int y_offset, int z_offset)
{
  for (int i = 0; i < path.size(); i++) {
    path[i].x += x_offset;
    path[i].y += y_offset;
    path[i].z += z_offset;
  }
}

Generic_map::Generic_map(int x, int y, int z)
{
  set_size(x, y, z);
  x_offset = 0;
  y_offset = 0;
  z_offset = 0;
}

Generic_map::~Generic_map()
{
}

void Generic_map::set_size(int x, int y, int z)
{
  cost.clear();
  if (x == 0 && y == 0) {
    return;
  }
  std::vector<int> tmpvec;
  std::vector<std::vector<int> > tmpmatrix;
  for (int i = 0; i < z; i++) {
    tmpvec.push_back(0);
  }
  for (int i = 0; i < y; i++) {
    tmpmatrix.push_back(tmpvec);
  }
  for (int i = 0; i < x; i++) {
    cost.push_back( tmpmatrix );
  }
}

void Generic_map::set_cost(int x, int y, int c)
{
  set_cost(x, y, 0, c);
}

void Generic_map::set_cost(int x, int y, int z, int c)
{
  if (x < 0 || x >= get_size_x() || y < 0 || y >= get_size_y() || z < 0 ||
      z >= get_size_z()) {
    return;
  }
  cost[x][y][z] = c;
}

void Generic_map::set_cost(Tripoint p, int c)
{
  set_cost(p.x, p.y, p.z, c);
}

int Generic_map::get_size_x()
{
  return cost.size();
}

int Generic_map::get_size_y()
{
  if (cost.empty()) {
    return 0;
  }
  return cost[0].size();
}

int Generic_map::get_size_z()
{
  if (cost.empty()) {
    return 0;
  }
  if (cost[0].empty()) {
    return 0;
  }
  return cost[0][0].size();
}

int Generic_map::get_cost(int x, int y, int z)
{
  if (x < 0 || x >= get_size_x() || y < 0 || y >= get_size_y() || z < 0 ||
      z >= get_size_z()) {
    return 0;
  }
  return cost[x][y][z];
}

int Generic_map::get_cost(Point p)
{
  return get_cost(p.x, p.y, 0);
}

int Generic_map::get_cost(Tripoint p)
{
  return get_cost(p.x, p.y, p.z);
}

bool Generic_map::blocked(int x, int y, int z)
{
  return (get_cost(x, y, z) <= 0);
}

bool Generic_map::blocked(Point p)
{
  return blocked(p.x, p.y);
}

bool Generic_map::blocked(Tripoint p)
{
  return blocked(p.x, p.y, p.z);
}

Pathfinder::Pathfinder()
{
  allow_diag = true;
  x_min  = 0;
  x_max  = 0;
  y_min  = 0;
  y_max  = 0;
  z_min  = 0;
  z_max  = 0;
  border = 0;
}

Pathfinder::Pathfinder(Generic_map m)
{
  allow_diag = true;
  set_map(m);
}

Pathfinder::~Pathfinder()
{
}

void Pathfinder::set_map(Generic_map m)
{
  map = m;
  set_bounds(0, 0, 0,
             map.get_size_x() - 1, map.get_size_y() - 1, map.get_size_z() - 1);
}

void Pathfinder::set_bounds(int x0, int y0, int x1, int y1)
{
  set_bounds(x0, y0, 0, x1, y1, 0);
}

void Pathfinder::set_bounds(int x0, int y0, int z0, int x1, int y1, int z1)
{

  if (x0 < 0) {
    x0 = 0;
  }
  if (x1 <= 0 || x1 >= map.get_size_x()) {
    x1 = map.get_size_x() - 1;
  }
  if (x0 > x1) {
    int tmp = x1;
    x1 = x0;
    x0 = tmp;
  }
  x_min = x0;
  x_max = x1;
  if (y0 < 0) {
    y0 = 0;
  }
  if (y1 <= 0 || y1 >= map.get_size_y()) {
    y1 = map.get_size_y() - 1;
  }
  if (y0 > y1) {
    int tmp = y1;
    y1 = y0;
    y0 = tmp;
  }
  y_min = y0;
  y_max = y1;
  if (z0 < 0) {
    z0 = 0;
  }
  if (z1 <= 0 || z1 >= map.get_size_z()) {
    z1 = map.get_size_z() - 1;
  }
  if (z0 > z1) {
    int tmp = z1;
    z1 = z0;
    z0 = tmp;
  }
  z_min = z0;
  z_max = z1;
}

void Pathfinder::set_bounds(Point p0, Point p1)
{
  set_bounds(p0.x, p0.y, p1.x, p1.y);
}

void Pathfinder::set_bounds(Tripoint p0, Tripoint p1)
{
  set_bounds(p0.x, p0.y, p0.z, p1.x, p1.y, p1.z);
}

void Pathfinder::set_bounds(int b)
{
  border = b;
}

void Pathfinder::set_allow_diagonal(bool allow)
{
  allow_diag = allow;
}

bool Pathfinder::in_bounds(int x, int y, int z)
{
  return (x >= x_min && x >= 0 && x <= x_max && x < map.get_size_x() &&
          y >= y_min && y >= 0 && y <= y_max && y < map.get_size_y() &&
          z >= z_min && z >= 0 && z <= z_max && z < map.get_size_z()   );
}

bool Pathfinder::in_bounds(Point p)
{
  return in_bounds(p.x, p.y);
}

bool Pathfinder::in_bounds(Tripoint p)
{
  return in_bounds(p.x, p.y, p.z);
}

Path Pathfinder::get_path(Path_type type, int x0, int y0, int x1, int y1)
{
  return get_path(type, Tripoint(x0, y0, 0), Tripoint(x1, y1, 0));
}

Path Pathfinder::get_path(Path_type type, Point start, Point end)
{
  return get_path(type, Tripoint(start.x, start.y, 0),
                        Tripoint(end.x, end.y, 0));
}

Path Pathfinder::get_path(Path_type type, int x0, int y0, int z0,
                                          int x1, int y1, int z1)
{
  return get_path(type, Tripoint(x0, y0, z0), Tripoint(x1, y1, z1));
}

Path Pathfinder::get_path(Path_type type, Tripoint start, Tripoint end)
{
  switch (type) {
    case PATH_NULL:
    case PATH_LINE:
      return path_line(start, end);
    case PATH_A_STAR:
      return path_a_star(start, end);
    default:
      return path_line(start, end);
  }
  return path_line(start, end);
}

Tripoint Pathfinder::get_step(Path_type type, int x0, int y0, int x1, int y1)
{
  return get_step(type, Tripoint(x0, y0, 0), Tripoint(x1, y1, 0));
}

Tripoint Pathfinder::get_step(Path_type type, Point start, Point end)
{
  return get_step(type, Tripoint(start.x, start.y, 0),
                        Tripoint(end.x, end.y, 0));
}

Tripoint Pathfinder::get_step(Path_type type, int x0, int y0, int z0,
                                              int x1, int y1, int z1)
{
  return get_step(type, Tripoint(x0, y0, z0), Tripoint(x1, y1, z1));
}

Tripoint Pathfinder::get_step(Path_type type, Tripoint start, Tripoint end)
{
  Path p = get_path(type, start, end);
  if (p.empty()) {
    return start;
  }
  return p[0];
}

Path Pathfinder::path_line(Tripoint start, Tripoint end)
{
  Path ret;
  Tripoint cur = start;
  bool done = false;
  while (!done) {
    bool picked_next = false;
    if (cur == end) {
      done = true;
// Prioritize vertical movement over lateral
    } else if (end.z < start.z && !map.blocked(cur.x, cur.y, cur.z - 1)) {
      picked_next = true;
      cur.z--;
    } else if (end.z > start.z && !map.blocked(cur.x, cur.y, cur.z + 1)) {
      picked_next = true;
      cur.z++;
    } else {
      Tripoint options[5];
      for (int i = 0; i < 5; i++) {
        options[i] = cur;
      }
      bool x_diff_bigger = ( abs(end.x - cur.x) > abs(end.y - cur.y) );
      int best_x_move = cur.x, alt_x_move = cur.x, worst_x_move = cur.x;;
      if (end.x > cur.x) {
        best_x_move++;
        worst_x_move--;
      } else if (end.x < cur.x) {
        best_x_move--;
        worst_x_move++;
      } else {
        int alt = 2 * rng(0, 1) - 1; // -1 or 1
        alt_x_move += alt;
        worst_x_move += -1 * alt;
      }
      int best_y_move = cur.y, alt_y_move = cur.y, worst_y_move = cur.y;;
      if (end.y > cur.y) {
        best_y_move++;
        worst_y_move--;
      } else if (end.y < cur.y) {
        best_y_move--;
        worst_y_move++;
      } else {
        int alt = 2 * rng(0, 1) - 1; // -1 or 1
        alt_y_move += alt;
        worst_y_move += -1 * alt;
      }
  
      options[0]   = Tripoint(best_x_move,  best_y_move,  cur.z);
      if (x_diff_bigger) {
        options[1] = Tripoint(best_x_move,  alt_y_move,   cur.z);
        options[2] = Tripoint(alt_x_move,   best_y_move,  cur.z);
        options[3] = Tripoint(best_x_move,  worst_y_move, cur.z);
        options[4] = Tripoint(worst_x_move, best_y_move,  cur.z);
      } else {
        options[1] = Tripoint(alt_x_move,   best_y_move,  cur.z);
        options[2] = Tripoint(best_x_move,  alt_y_move,   cur.z);
        options[3] = Tripoint(worst_x_move, best_y_move,  cur.z);
        options[4] = Tripoint(best_x_move,  worst_y_move, cur.z);
      }
      for (int i = 0; i < 5 && !picked_next; i++) {
        if (!map.blocked( options[i] ) && in_bounds( options[i] )) {
          picked_next = true;
          cur = options[i];
        }
      }
    } // Lateral movement
    if (!picked_next) { // Couldn't reach our target using this stupid algo!
      done = true;
    } else {
      ret.add_step(cur, map.get_cost(cur));
      if (cur.x == end.x && cur.y == end.y) {
        cur = end;
        ret.add_step(cur, map.get_cost(cur));
      }
    }
  } // while (!done)

  if (cur != end) { // We didn't make it :(
    return Path();
  }
  return ret;
}


enum A_star_status
{
  ASTAR_NONE,
  ASTAR_OPEN,
  ASTAR_CLOSED
};

Path Pathfinder::path_a_star(Tripoint start, Tripoint end)
{
  int x_size = map.get_size_x();
  int y_size = map.get_size_y();
  int z_size = map.get_size_z();

  start.x -= map.x_offset;
  start.y -= map.y_offset;
  start.z -= map.z_offset;
  end.x   -= map.x_offset;
  end.y   -= map.y_offset;
  end.z   -= map.z_offset;

  if (x_size == 0 || y_size == 0 || z_size == 0) {
    debugmsg("A* generated; %s => %s (size %d, %d, %d)",
             start.str().c_str(), end.str().c_str(),
             x_size, y_size, z_size);
    return Path();
  }

  std::vector<Tripoint> open_points;
  A_star_status status[x_size][y_size][z_size];
  int           gscore[x_size][y_size][z_size];
  int           hscore[x_size][y_size][z_size];
  Tripoint      parent[x_size][y_size][z_size];

  if (border > 0) {
    int x0 = (start.x < end.x ? start.x : end.x);
    int y0 = (start.y < end.y ? start.y : end.y);
    int z0 = (start.z < end.z ? start.z : end.z);
    int x1 = (start.x > end.x ? start.x : end.x);
    int y1 = (start.y > end.y ? start.y : end.y);
    int z1 = (start.z > end.z ? start.z : end.z);

    set_bounds(x0 - border, y0 - border, z0 - border,
               x1 + border, y1 + border, z1 + border);
  }

// Init everything to 0
  for (int x = 0; x < x_size; x++) {
    for (int y = 0; y < y_size; y++) {
      for (int z = 0; z < z_size; z++) {
        status[x][y][z] = ASTAR_NONE;
        gscore[x][y][z] = 0;
        hscore[x][y][z] = 0;
        parent[x][y][z] = Tripoint(-1, -1, -1);
      }
    }
  }

  status[start.x][start.y][start.z] = ASTAR_OPEN;
  open_points.push_back(start);

  bool done = false;


  while (!done && !open_points.empty()) {
// 1) Find the lowest cost in open_points:
    int lowest_cost = -1, point_index = -1;
    Tripoint current;
    int current_g = 0;
    for (int i = 0; i < open_points.size(); i++) {
      Tripoint p = open_points[i];
      int score = gscore[p.x][p.y][p.z] + hscore[p.x][p.y][p.z];
      if (i == 0 || score < lowest_cost) {
        lowest_cost = score;
        current = p;
        current_g = gscore[p.x][p.y][p.z];
        point_index = i;
      }
    }
// 2) Check if that's the endpoint
    if (current == end) {
      done = true;
    } else {
// 3) Set that point to be closed
      open_points.erase(open_points.begin() + point_index);
      status[current.x][current.y][current.z] = ASTAR_CLOSED;
// 4) Examine all adjacent points on the same z-level
      for (int x = current.x - 1; x <= current.x + 1; x++) {
        for (int y = current.y - 1; y <= current.y + 1; y++) {
          if (x == current.x && y == current.y) {
            y++; // Skip the current tile
          }
          int z = current.z;
// If it's no-diagonal or diagonals are allowed...
// ...and if it's in-bounds and not blocked...
          if ((allow_diag || x == current.x || y == current.y) &&
              (in_bounds(x, y, z) && !map.blocked(x, y, z))) {
            int g = current_g + map.get_cost(x, y, z);
// If it's unexamined, make it open and set its values
            if (status[x][y][z] == ASTAR_NONE) {
              status[x][y][z] = ASTAR_OPEN;
              gscore[x][y][z] = g;
              if (allow_diag) {
                hscore[x][y][z] = map.get_cost(x, y, z) *
                                  rl_dist(x, y, z, end.x, end.y, end.z);
              } else {
                hscore[x][y][z] = map.get_cost(x, y, z) *
                                  manhattan_dist(x, y, z, end.x, end.y, end.z);
              }
              parent[x][y][z] = current;
              open_points.push_back( Tripoint(x, y, z) );
// If it's open and we're a better parent, make us the parent
            } else if (status[x][y][z] == ASTAR_OPEN && g < gscore[x][y][z]) {
              gscore[x][y][z] = g;
              parent[x][y][z] = current;
            }
          }
        }
      }
// 5.  Examine adjacent points on adjacent Z-levels
// TODO: Allow diagonal movement across Z-levels?
      for (int z = current.z - 1; z <= current.z + 1; z++) {
        int x = current.x, y = current.y;
        if ((in_bounds(x, y, z) && !map.blocked(x, y, z))) {
          int g = current_g + map.get_cost(x, y, z);
// If it's unexamined, make it open and set its values
          if (status[x][y][z] == ASTAR_NONE) {
            status[x][y][z] = ASTAR_OPEN;
            gscore[x][y][z] = g;
            if (allow_diag) {
              hscore[x][y][z] = map.get_cost(x, y, z) *
                                rl_dist(x, y, z, end.x, end.y, end.z);
            } else {
              hscore[x][y][z] = map.get_cost(x, y, z) *
                                manhattan_dist(x, y, z, end.x, end.y, end.z);
            }
            parent[x][y][z] = current;
            open_points.push_back( Tripoint(x, y, z) );
// If it's open and we're a better parent, make us the parent
          } else if (status[x][y][z] == ASTAR_OPEN && g < gscore[x][y][z]) {
            gscore[x][y][z] = g;
            parent[x][y][z] = current;
          }
        }
      }
    }
  }

  Path ret;
  if (open_points.empty()) {
    return ret;
  }
  Tripoint cur = end;
  ret.add_step(cur, map.get_cost(cur));
  while (parent[cur.x][cur.y][cur.z] != start) {
    cur = parent[cur.x][cur.y][cur.z];
    ret.add_step(cur, map.get_cost(cur));
  }
  ret.reverse();
// Add the offsets back in.
  ret.offset(map.x_offset, map.y_offset, map.z_offset);

  return ret;
}
