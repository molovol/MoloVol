#ifndef MISC_H

#define MISC_H

#include <string>
#include <array>
#include <cmath>
#include <bitset>
#include <iostream>

inline double distance(const std::array<double,3> &start, const std::array<double,3> &end){
  return std::pow( (pow(end[0]-start[0],2) + pow(end[1]-start[1],2) + pow(end[2]-start[2],2)) , 0.5);
}

inline double distance(const std::array<double,3> &start, const std::array<double,3> &end, const int dim){
  return end[dim]-start[dim];
}

std::string fileExtension(const std::string& path);

template <typename T>
void print(std::array<T,3> arr){
  for (T elem : arr){
    std::cout << elem << " ";
  }
  std::cout << std::endl;
}

template <typename T>
void swap(T& i, T& j){
  int temp = i;
  i = j;
  j = temp;
}

template <typename T1, typename T2>
std::array<T1,3> add(std::array<T1,3> arr1, const std::array<T2,3>& arr2){
  for (char i = 0; i < 3; i++){
    arr1[i] += arr2[i];
  }
  return arr1;
}

////////////////////
// BIT OPERATIONS //
////////////////////

template <typename T>
void printBinary(const T var){
  try {
    std::bitset<sizeof(var) * CHAR_BIT> bit(var);
    std::cout << bit << std::endl;
  }
  catch (std::invalid_argument& e) {
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
