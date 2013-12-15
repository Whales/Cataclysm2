#include <sstream>
#include "glyph.h"
#include "options.h"

std::string glyph::save_data()
{
 std::stringstream ret;
 ret << symbol << " " << int(fg) << " " << int(bg);
 return ret.str();
};

void glyph::load_data(std::istream &datastream)
{
 int fgtmp, bgtmp;
 datastream >> symbol >> fgtmp >> bgtmp;

 fg = nc_color(fgtmp);
 bg = nc_color(bgtmp);
}

glyph glyph::invert()
{
  glyph ret = (*this);
  nc_color tmp = ret.fg;
  ret.fg = ret.bg;
  ret.bg = tmp;
  if (NO_BRIGHT_BG) {
    ret.bg = non_bright(ret.bg);
  }
  return ret;
}

glyph glyph::hilite()
{
  if (bg == HILITE_COLOR) {
    return invert();
  }
  glyph ret = (*this);
  
  ret.bg = HILITE_COLOR;
  return ret;
}

std::string glyph::text_formatted()
{
  std::stringstream ret;
  ret << "<c=" << color_tag(fg) << "," << color_tag(bg) << ">" <<
         char(symbol) << "<c=/>";
  return ret.str();
}

bool glyph::operator==(const glyph &rhs)
{
 return (rhs.fg == fg && rhs.bg == bg && rhs.symbol == symbol);
}
