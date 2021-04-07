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

// find the current time and convert to a string in format year-month-day_hour-min-sec
std::string timeNow(){
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,80,"%Y-%m-%d_%Hh%Mm%Ss",timeinfo);

    return buffer;
}

int pow2(int exp){
  return (1 << exp);
}
