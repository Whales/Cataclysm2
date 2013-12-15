#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include "files.h"

std::vector<std::string> files_in(std::string dir, std::string suffix)
{
  std::vector<std::string> ret;
  DIR *dp;
  dirent *dirp;

  if ( (dp = opendir(dir.c_str())) == NULL )
    return ret;

  while ( (dirp = readdir(dp)) != NULL ) {
    std::string filename(dirp->d_name);
    if (suffix == "" || filename.find(suffix) != std::string::npos) {
      ret.push_back( std::string(dirp->d_name) );
    }
  }

  closedir(dp);
  return ret;
}

std::vector<std::string> directories_in(std::string dir)
{
  std::vector<std::string> ret;
  DIR *dp;
  dirent *dirp;

  if ( (dp = opendir(dir.c_str())) == NULL ) {
    return ret;
  }

  while ( (dirp = readdir(dp)) != NULL ) {
    if (dirp->d_type == DT_DIR) {
      std::string dname = dirp->d_name;
      if (dname[0] != '.') {
        ret.push_back( std::string(dirp->d_name) );
      }
    }
  }
  return ret;
}

std::string slurp_file(const std::string &filename)
{
  std::string ret;
  std::ifstream fin;
  fin.open(filename.c_str());
  if (!fin.is_open()) {
    return ret;
  }

  ret.assign( (std::istreambuf_iterator<char>(fin) ),
              (std::istreambuf_iterator<char>()    ) );

  return ret;
}
