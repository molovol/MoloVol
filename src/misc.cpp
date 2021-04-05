#include "misc.h"

std::string fileExtension(const std::string& path){
  // will cause an issue, if there is a dot in the middle of the file AND no file extension
  std::string after_dot = "";
  size_t dot_pos = path.find_last_of(".");
  if (dot_pos != std::string::npos){
    return path.substr(dot_pos+1);
  }
  else{
    return "invalid";
  }
}

int pow2(int exp){
  return (1 << exp);
}
