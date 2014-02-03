#include "glyph.h"
#include "options.h"
#include "window.h"
#include <sstream>

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

void glyph::load_data_text(std::istream &datastream, std::string owner_name)
{
  std::string fgtmp, bgtmp;
  char tmpch;
  datastream >> tmpch >> fgtmp >> bgtmp;

  symbol = tmpch;
  fg = color_string(fgtmp);
  if (fg == c_null) {
    debugmsg("Loaded bad color '%s' (%s)", fgtmp.c_str(), owner_name.c_str());
  }
  bg = color_string(bgtmp);
  if (bg == c_null) {
    debugmsg("Loaded bad color '%s' (%s)", fgtmp.c_str(), owner_name.c_str());
  }
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

glyph glyph::hilite(nc_color back)
{
  if (fg == back) {
    return invert();
  }
  glyph ret = (*this);
  
  ret.bg = back;
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

void glyph::make_line_drawing(bool north, bool east, bool south, bool west)
{
  if (north) {
    if (east) {
      if (south) {
        if (west) {
          symbol = LINE_XXXX;
        } else {
          symbol = LINE_XXXO;
        }
      } else {
        if (west) {
          symbol = LINE_XXOX;
        } else {
          symbol = LINE_XXOO;
        }
      }
    } else {
      if (south) {
        if (west) {
          symbol = LINE_XOXX;
        } else {
          symbol = LINE_XOXO;
        }
      } else {
        if (west) {
          symbol = LINE_XOOX;
        } else {
          symbol = LINE_XOXO;
        }
      }
    }
  } else {
    if (east) {
      if (south) {
        if (west) {
          symbol = LINE_OXXX;
        } else {
          symbol = LINE_OXXO;
        }
      } else {
        if (west) {
          symbol = LINE_OXOX;
        } else {
          symbol = LINE_OXOX;
        }
      }
    } else {
      if (south) {
        if (west) {
          symbol = LINE_OOXX;
        } else {
          symbol = LINE_XOXO;
        }
      } else {
        if (west) {
          symbol = LINE_OXOX;
        } else {
          symbol = LINE_XXXX;
        }
      }
    }
  }
}
