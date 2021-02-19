#ifndef CONTAINER3D_H

#define CONTAINER3D_H

#include <iostream>
#include <cassert>
#include <array>
#include <vector>

template <class T>
class Container3D{
  public:
    Container3D(const unsigned int x, const unsigned int y, const unsigned int z){
      _data = std::vector(x*y*z,T());
      _n_elements[0] = x;
      _n_elements[1] = y;
      _n_elements[2] = z;
    }
    Container3D(const std::array<unsigned int,3> steps){
      _data = std::vector(steps[0]*steps[1]*steps[2],T());
      _n_elements = steps;
    }

    T& getElement(const unsigned int x, const unsigned int y, const unsigned int z){
      // check if element is out of bounds
      assert(x < _n_elements[0]);
      assert(y < _n_elements[1]);
      assert(z < _n_elements[2]);
      return _data[z * _n_elements[0] * _n_elements[1] + y * _n_elements[0] + x];
    }

    T& getElement(const std::array<unsigned int,3> coord){
      // check if element is out of bounds
      for (char i = 0; i < 3; i++){
        assert(coord[i] < _n_elements[i]);
      }
      return _data[coord[2] * _n_elements[0] * _n_elements[1] + coord[1] * _n_elements[0] + coord[0]];
    }

    std::array<unsigned int,3> getNumElements() const {
      return _n_elements;
    }
   
    void print(){
      for (unsigned int i = 0; i<_n_elements[0]; i++){
        for (unsigned int j = 0; j<_n_elements[1]; j++){
          for (unsigned int k = 0; k<_n_elements[2]; k++){
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
    std::array<unsigned int,3> _n_elements;
};

#endif
