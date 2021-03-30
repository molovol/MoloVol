#ifndef CONTAINER3D_H

#define CONTAINER3D_H

#include <iostream>
#include <cassert>
#include <array>
#include <vector>

template <class T>
class Container3D{
  public:
    // constructors
    Container3D() = default;

    Container3D(const unsigned long int x, const unsigned long int y, const unsigned long int z){
      _data = std::vector(x*y*z,T());
      _n_elements[0] = x;
      _n_elements[1] = y;
      _n_elements[2] = z;
    }

    Container3D(const std::array<unsigned long int,3> steps){
      _data = std::vector(steps[0]*steps[1]*steps[2],T());
      _n_elements = steps;
    }
    
    Container3D(const std::array<unsigned int,3> steps) 
      : Container3D((unsigned long) steps[0], (unsigned long) steps[1], (unsigned long) steps[2]){}
    
    // get element
    // single integer
    T& getElement(const unsigned long int i){return _data[i];}
    // three integers
    T& getElement(const unsigned long int x, const unsigned long int y, const unsigned long int z){
      // check if element is out of bounds
//      assert(x < _n_elements[0]);
//      assert(y < _n_elements[1]);
//      assert(z < _n_elements[2]);
      return _data[z * _n_elements[0] * _n_elements[1] + y * _n_elements[0] + x];
    }
    // arrays
    T& getElement(const std::array<unsigned long int,3> coord){
      return _data[coord[2] * _n_elements[0] * _n_elements[1] + coord[1] * _n_elements[0] + coord[0]];}
    
    T& getElement(const std::array<unsigned int,3> coord){
      return _data[coord[2] * _n_elements[0] * _n_elements[1] + coord[1] * _n_elements[0] + coord[0]];}
    
    T& getElement(const std::array<long int,3> coord){
      return _data[coord[2] * _n_elements[0] * _n_elements[1] + coord[1] * _n_elements[0] + coord[0]];}
    
    T& getElement(const std::array<int,3> coord){
      return _data[coord[2] * _n_elements[0] * _n_elements[1] + coord[1] * _n_elements[0] + coord[0]];}
    // get boundaries
    std::array<unsigned long int,3> getNumElements() const {
      return _n_elements;
    }
    // display 
    void print(){
      for (unsigned long int i = 0; i<_n_elements[0]; i++){
        for (unsigned long int j = 0; j<_n_elements[1]; j++){
          for (unsigned long int k = 0; k<_n_elements[2]; k++){
            // TODO: find a general solution for outputting contents
            if (getElement(i,j,k) == 0b00000011){
              std::cout << "A" << " ";
            }
            else{
              std::cout << "e" << " ";}
          }
          std::cout << std::endl;
        }
        std::cout << std::endl;
      }
    }
  
  private:
    std::vector<T> _data;
    std::array<unsigned long int,3> _n_elements;
};

#endif
