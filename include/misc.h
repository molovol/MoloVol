#ifndef MISC_H

#define MISC_H

#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <climits>
#include <bitset>
#include <ctime>
#include <iostream>
#include <filesystem>

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#endif

std::string getResourcesDir();

template <typename T>
inline bool isIncluded(const T& elem, const std::vector<T>& list) {
  return (std::find(list.begin(), list.end(), elem) != list.end());
}

inline double distance(const std::array<double,3> &start, const std::array<double,3> &end){
  return std::pow( (pow(end[0]-start[0],2) + pow(end[1]-start[1],2) + pow(end[2]-start[2],2)) , 0.5);
}

inline double distance(const std::array<double,3> &start, const std::array<double,3> &end, const int dim){
  return end[dim]-start[dim];
}

std::string fileExtension(const std::string& path);

std::string fileName(const std::string& path);

std::string timeNow();

template <typename T>
void print(std::array<T,3> arr){
  for (T elem : arr){
    std::cout << elem << " ";
  }
  std::cout << std::endl;
}

template <typename T1, typename T2>
std::array<T1,3> add(std::array<T1,3> arr1, const std::array<T2,3>& arr2){
  for (char i = 0; i < 3; i++){
    arr1[i] += arr2[i];
  }
  return arr1;
}

int pow2(int);

namespace StrMngr {
  void removeEOL(std::string& str);
  void removeWhiteSpaces(std::string& str);
  void extendToLength(std::string& str, size_t);
}

inline double custom_fmod(double numer, double denom){
  return (numer - int((numer/denom)) * denom);
}

std::string field(int n_ws, std::string="", char='l');
std::wstring wfield(int n_ws, std::wstring=L"", char='l');

////////////////////
// BIT OPERATIONS //
////////////////////

template <typename T>
void printBinary(const T var){
  try {
    std::bitset<sizeof(var) * CHAR_BIT> bit(var);
    std::cout << bit << std::endl;
  }
  catch (const std::invalid_argument& e) {
    throw;
  }
}

template <typename T>
void setBitOn(T& var, const unsigned pos){
  var |= 1 << pos;
}

template <typename T>
void setBitOff(T& var, const unsigned pos){
  var &= ~(1 << pos);
}

template <typename T>
void setBit(T& var, const unsigned pos, const bool state){
  if (state){
    setBitOn(var,pos);
  }
  else {
    setBitOff(var,pos);
  }
}

template <typename T>
inline bool readBit(const T var, const unsigned pos){
  return (var & ( 1 << pos )) >> pos;
}

#endif
