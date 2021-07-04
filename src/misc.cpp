#include "misc.h"
#include <sstream>
#include <iomanip>
#include <experimental/filesystem>

// access the resource folder containing elements.txt and space_groups.txt
std::string getResourcesDir(){
#if defined(__APPLE__) && !defined(DEBUG)
  CFURLRef resource_url = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
  char resource_path[PATH_MAX];
  if (CFURLGetFileSystemRepresentation(resource_url, true, (UInt8 *)resource_path, PATH_MAX)){
    if (resource_url != NULL){
      CFRelease(resource_url);
    }
    return resource_path;
  }
  return "";
#elif defined(__linux__) && !defined(DEBUG)
  return "/usr/share/molovol";
#else
  return "./inputfile";
#endif
}

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
  std::array<char,2> separators = {'\\','/'};
  for (char sep : separators){
    size_t sep_pos = path.find_last_of(sep);
    if (sep_pos != std::string::npos){
      return path.substr(sep_pos+1);
    }
  }
  return "invalid";
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

void removeEOL(std::string& str){
  str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

void removeWhiteSpaces(std::string& str){
  str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
}

std::string field(int n_ws, std::string text, char alignment){
  std::stringstream ss;
  if (alignment == 'r'){
    ss << std::right;
  }
  else if (alignment == 'i'){
    ss << std::internal;
  }
  else{
    ss << std::left;
  }
  ss << std::setw(n_ws) << text;
  return ss.str();
};

std::wstring wfield(int n_ws, std::wstring text, char alignment){
  std::wstringstream ss;
  if (alignment == 'r'){
    ss << std::right;
  }
  else if (alignment == 'i'){
    ss << std::internal;
  }
  else{
    ss << std::left;
  }
  ss << std::setw(n_ws) << text;
  return ss.str();
};

