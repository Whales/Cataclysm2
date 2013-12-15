#include "stringfunc.h"

std::vector<std::string> break_into_lines(std::string text, int linesize)
{
 std::vector<std::string> ret;

 size_t chars = 0; // Number of actually-printed characters at...
 size_t pos = 0; // ... this point in the string
 size_t linebreak = std::string::npos; // The last acceptable breakpoint
 std::string active_color_tag;
 while (text.length() > linesize && pos < text.size()) {
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
 ret.push_back(text);
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
         (ret[front] == ' ' || ret[front] == '\n' || ret[front] == '\t'))
    front++;

  ret = ret.substr(front);

  back = ret.length() - 1;

  while (back >= 0 &&
         (ret[back] == ' ' || ret[back] == '\n' || ret[back] == '\t'))
    back--;

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
