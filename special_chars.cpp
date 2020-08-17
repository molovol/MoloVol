
#include "special_chars.h"
#include <iostream>

std::string Symbol::angstrom(){
  return "\xE2\x84\xAB";
}

std::string Symbol::cubed(){
  return "\xC2\xB3";
}
// TODO: write function to return subscript encoding based on int or char

// from an int, produce a string of utf-8 encoded subscripts
std::string Symbol::subscript(int num){  
  std::string retval = "";

  while (num > 0){
    char subscript[4] = "\xE2\x82\x80"; // utf-8 encoding of subscript "0"
    int digit = num%10;
    subscript[2] += digit;
    for (char c : subscript){
      retval.push_back(c);
    }
    num = num/10;
  }
  return retval;
}

// from a numeric string, produce a string of utf-8 encoded subscripts
std::string Symbol::subscript(std::string num){
  std::string retval = "";
  
  // iterate over all chars in input
  for (char c : num){ 
    assert(c >= '0' && c <= '9');
    char subscript[4] = "\xE2\x82\x80";   // utf-8 encoding of subscript "0"
    int digit = c - '0';                  // covert numeric char to corresponding int
    subscript[2] += digit;                // adding a digit to subscript "0" return subscript of that digit
    for (char c : subscript){
      retval.push_back(c);
    }

  }
  return retval;
}


