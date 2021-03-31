#include "misc.h"

std::string fileExtension(const std::string& path){
  // will cause an issue, if there is a dot in the middle of the file AND no file extension
  size_t dot_pos = path.find_last_of(".");
  if (dot_pos != std::string::npos){
    return path.substr(dot_pos+1);
  }
  else{
    return "invalid";
  }
}

std::string fileName(const std::string& path){
  size_t bslash_pos = path.find_last_of("\\");
  if (bslash_pos != std::string::npos){
    return path.substr(bslash_pos+1);
  }
  else{
    return "invalid";
  }
}

