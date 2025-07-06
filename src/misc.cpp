#include "misc.h"
#include <sstream>
#include <iomanip>

#if defined _WIN32
#include <windows.h>
#endif

// access the resource folder containing elements.txt and space_groups.txt
// Because std::filesystem was only introduced to macOS in 10.15 (released 2020) I have
// decided to reimplement the method using std::string as return type. Consider reverting
// this change once backwards compatibility to pre-10.15 macOS is no longer needed.
std::string getResourcesDir() {
  //std::filesystem::path res_dir = "";
  std::string res_dir = "";
#if defined(__APPLE__) && defined(ABS_PATH)
    CFURLRef resource_url = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    char resource_path[PATH_MAX];
    if (CFURLGetFileSystemRepresentation(resource_url, true, (UInt8*)resource_path, PATH_MAX)) {
        if (resource_url != NULL) {
            CFRelease(resource_url);
        }
        //res_dir = std::filesystem::path(resource_path);
        res_dir = resource_path;
    }

#elif defined(__linux__) && defined(ABS_PATH)
    //res_dir = std::filesystem::path("/usr/share/molovol");
    res_dir = "/usr/share/molovol";

#elif defined(_WIN32) && defined(ABS_PATH)
    std::filesystem::path res_dir_path = "";
    wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(NULL, buffer, MAX_PATH);

    if (length) {
      std::wstring executablePath(buffer);
      size_t pos = executablePath.find_last_of(L"\\");

      if (pos != std::wstring::npos) {
          std::wstring executableDir = executablePath.substr(0, pos);
          std::filesystem::path executableDirPath(executableDir.begin(), executableDir.end());
          res_dir_path = executableDirPath / ".." / "shared" / "inputfile";
      }
    }
    res_dir = res_dir_path.string();

#else
    //res_dir = std::filesystem::path("./inputfile");
    res_dir = "./inputfile";
#endif
    //return res_dir.lexically_normal();
    return res_dir;
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
      return path.substr(sep_pos+1,path.find_last_of('.')-(sep_pos+1));
    }
  }
  return "invalid";
}

// find the current time and convert to a string in format year-month-day_hour-min-sec
std::string timeNow(){
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d_%Hh%Mm%Ss");
    return oss.str();
}

int pow2(int exp){
  return (1 << exp);
}

namespace StrMngr {
  void removeEOL(std::string& str){
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  }
  
  void removeWhiteSpaces(std::string& str){
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
  }
  
  void extendToLength(std::string& str, size_t len){
    if (len <= str.size()){return;}
    str += std::string(len - str.size(), ' ');
  }
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

