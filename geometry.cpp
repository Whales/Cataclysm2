#include <vector>
#include <stdlib.h>
#include "geometry.h"

#define SGN(a) (((a)<0) ? -1 : 1)

std::vector <point> line_to(int x0, int y0, int x1, int y1)
{
 int t = 0;
 std::vector<point> ret;
 int dx = x1 - x0;
 int dy = y1 - y0;
 int ax = abs(dx)<<1;
 int ay = abs(dy)<<1;
 int sx = SGN(dx);
 int sy = SGN(dy);
 if (dy == 0) sy = 0;
 if (dx == 0) sx = 0;
 point cur;
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


int rl_dist(int x0, int y0, int x1, int y1)
{
  int dx = (x0 > x1 ? x0 - x1 : x1 - x0), dy = (y0 > y1 ? y0 - y1 : y1 - y0);
  return (dx > dy ? dx : dy);
}
