#ifndef _STRINGFUNC_H_
#define _STRINGFUNC_H_

#include <string>
#include <vector>
#include <istream>

#define STD_DELIM "</>"

std::vector<std::string> break_into_lines(std::string text, int linesize);

std::string load_to_delim(std::istream &datastream, std::string delim);
std::string load_to_character(std::istream &datastream, char ch,
                              bool _trim = false);
std::string load_to_character(std::istream &datastream, std::string chars,
                              bool _trim = false);

std::string slurp_file(const std::string &filename);

std::string trim(const std::string &orig);

std::string all_caps(const std::string &orig);
std::string no_caps (const std::string &orig);

#endif
