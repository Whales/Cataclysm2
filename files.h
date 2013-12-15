#ifndef _FILES_H_
#define _FILES_H_

#include <vector>
#include <string>

std::vector<std::string> files_in(std::string dir, std::string suffix = "");
std::vector<std::string> directories_in(std::string dir);

std::string slurp_file(const std::string &filename);

#endif
