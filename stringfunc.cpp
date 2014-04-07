#include <sstream>
#include "stringfunc.h"
#include "window.h"

std::vector<std::string> break_into_lines(std::string text, int linesize)
{
 std::vector<std::string> ret;

 size_t chars = 0; // Number of actually-printed characters at...
 size_t pos = 0; // ... this point in the string
 size_t linebreak = std::string::npos; // The last acceptable breakpoint
 std::string active_color_tag;
 while ((text.length() > linesize || text.find('\n') != std::string::npos) &&
        pos < text.size()) {
  bool force = false;
  if (text.substr(pos, 3) == "<c=") {
   size_t tmppos = text.find('>', pos);
   if (tmppos == std::string::npos) {
    //debugmsg("Bad colortag!");
    return ret;
   } else {
    active_color_tag = text.substr(pos, tmppos - pos + 1);
    pos = tmppos;
   }
   linebreak = pos;
   chars--;
  } else if (text[pos] == '\n') {
   linebreak = pos;
   force = true;
  } else if (text[pos] == ' ')
   linebreak = pos;
  pos++;
  chars++;
  if (force || chars > linesize) {
   std::string tmp;
   if (linebreak == std::string::npos) {
    linebreak = linesize - 1;
    tmp = text.substr(0, linebreak) + "-";
    text = text.substr(linebreak);
   } else if (text[linebreak] == '\n' || text[linebreak] == ' ') {
    tmp = text.substr(0, linebreak);
    text = text.substr(linebreak + 1);
// Ostensibly for color tags, but could cause a problem?
   } else if (text[linebreak] == '>') {
    linebreak++;
    tmp = text.substr(0, linebreak);
    text = text.substr(linebreak);
   }
   ret.push_back(tmp);
   if (!active_color_tag.empty()) {
    text = active_color_tag + text;
   }
   pos = 0;
   chars = 0;
   linebreak = std::string::npos;
  }
 }
 if (!text.empty()) {
  ret.push_back(text);
 }
 return ret;
/*
  size_t linebreak = text.find_last_of(" ", linesize);
  std::string tmp;
  if (linebreak == std::string::npos) {
   linebreak = linesize - 1;
   tmp = text.substr(0, linebreak) + "-";
  } else
   tmp = text.substr(0, linebreak);

  ret.push_back(tmp);
  text = text.substr(linebreak + 1);
 }

 ret.push_back(text);
*/

 return ret;
}

std::string load_to_delim(std::istream &datastream, std::string delim)
{
 std::string ret, tmp;
 do {
  datastream >> tmp;
  if (tmp != delim)
   ret += tmp + " ";
 } while (tmp != delim && !(datastream.eof()));

 if (!ret.empty() && ret[ret.size() - 1] == ' ')
  ret = ret.substr(0, ret.size() - 1);

 return ret;
}

std::string load_to_character(std::istream &datastream, char ch, bool _trim)
{
  std::string ret;
  char *tmp = new char [1];
  do {
    datastream.read(tmp, 1);
    if (tmp[0] != ch) {
      ret.push_back(tmp[0]);
    }
    debugmsg(ret.c_str());
  } while (tmp[0] != ch && !(datastream.eof()));

  if (_trim)
    ret = trim(ret);
  delete [] tmp;
  return ret;
}

std::string load_to_character(std::istream &datastream, std::string chars,
                              bool _trim)
{
  std::string ret;
  char *tmp = new char [1];
  do {
    datastream.read(tmp, 1);
    if (chars.find(tmp[0]) == std::string::npos) {
      ret.push_back(tmp[0]);
    }
    debugmsg(ret.c_str());
  } while (chars.find(tmp[0]) == std::string::npos && chars[0] >= 0 &&
           !(datastream.eof()));

  if (_trim)
    ret = trim(ret);

  delete [] tmp;
  return ret;
}

std::string trim(const std::string &orig)
{
  std::string ret = orig;
  int front = 0, back = ret.length() - 1;
  while (front < ret.length() &&
         (ret[front] == ' ' || ret[front] == '\n' || ret[front] == '\t')) {
    front++;
  }

  ret = ret.substr(front);

  back = ret.length() - 1;

  while (back >= 0 &&
         (ret[back] == ' ' || ret[back] == '\n' || ret[back] == '\t')) {
    back--;
  }

  ret = ret.substr(0, back + 1);

  return ret;
}

std::string all_caps(const std::string &orig)
{
  std::string ret = orig;
  for (int i = 0; i < ret.length(); i++) {
    if (ret[i] >= 'a' && ret[i] <= 'z')
      ret[i] += 'A' - 'a';
  }

  return ret;
}

std::string no_caps(const std::string &orig)
{
  std::string ret = orig;
  for (int i = 0; i < ret.length(); i++) {
    if (ret[i] >= 'A' && ret[i] <= 'Z')
      ret[i] += 'a' - 'A';
  }

  return ret;
}

std::string capitalize(const std::string &orig)
{
  std::string ret = orig;
  size_t tagpos = orig.find("<c="); // Find the first tag
  size_t start; // Used below
  if (tagpos != std::string::npos) {
    for (int i = 0; i < tagpos; i++) {  // Can we capitalize before the tag?
      if (ret[i] >= 'a' && ret[i] <= 'z') {
        ret[i] += 'A' - 'a';  // Capitalize!
        return ret;
      } else if (ret[i] != ' ') {
        return ret; // We're already capitalized!
      }
    }
// If we reach this point, we found a tag but there's nothing before it.
    start = orig.find(">", tagpos);
    start++;
  } else {  // No tags - start from the beginning of the string
    start = 0;
  }
  for (int i = start; i < ret.size(); i++) {
    if (ret[i] >= 'a' && ret[i] <= 'z') {
      ret[i] += 'A' - 'a';
      return ret;
    } else if (ret[i] != ' ') {
      return ret; // We're already capitalized!
    }
  }
  return ret; // All blank spaces??
}

std::string remove_color_tags(const std::string &orig)
{
  std::string ret;
  bool in_tag = false;
  for (int i = 0; i < orig.size(); i++) {
    if (in_tag) {
      if (orig[i] == '>') {
        in_tag = false;
      }
    } else {
      if (orig[i] == '<') {
        in_tag = true;
      } else {
        ret += orig[i];
      }
    }
  }
  return ret;
}

std::string itos(int num)
{
  std::stringstream ret;
  ret << num;
  return ret.str();
}

std::string color_gradient(int value, std::vector<int> breakpoints,
                           std::vector<nc_color> colors)
{
/* We need one more color than breakpoints, since breakpoints are the spots
 * BETWEEN colors:
 *  gray  <split: 5>  white  <split: 12>  green
 */
  if (breakpoints.size() + 1 != colors.size()) {
    debugmsg("color_gradient() called with mismatched breakpoints/colors!");
    return "";
  }

  nc_color col = c_null;
  for (int i = 0; i < breakpoints.size(); i++) {
    if (value <= breakpoints[i]) {
      col = colors[i];
    }
  }
  if (col == c_null) {
    col = colors.back();
  }
  std::stringstream ret;
  ret << "<c=" << color_tag(col) << ">";
  return ret.str();
}

bool is_vowel(char ch)
{
  return (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' ||
          ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U');
}
