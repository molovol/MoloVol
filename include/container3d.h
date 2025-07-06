#ifndef CONTAINER3D_H

#define CONTAINER3D_H

#include <iostream>
#include <cassert>
#include <array>
#include <vector>

template <class T>
class Container3D{
  public:
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
    
    /////////////
    // GETTERS //
    /////////////
    // Single index getter
    T& getElement(const unsigned long int i){return _data[i];}

    // XYZ index getter
    template<std::integral INT>
    T& getElement(const INT x, const INT y, const INT z){
      return _data[z * _n_elements[0] * _n_elements[1] + y * _n_elements[0] + x];
    }

    template<std::integral INT>
    const T& getElement(const INT x, const INT y, const INT z) const {
      return _data[z * _n_elements[0] * _n_elements[1] + y * _n_elements[0] + x];
    }

    // These functions should be a template function
    template<std::integral INT>
    T& getElement(const std::array<INT,3> coord){
      return _data[coord[2] * _n_elements[0] * _n_elements[1] + coord[1] * _n_elements[0] + coord[0]];
    }

    template <typename Q = unsigned long>
    std::array<Q,3> getNumElements() const {
      std::array<Q,3> arr;
      for (char i = 0; i < 3; ++i){
        arr[i] = static_cast<Q>(_n_elements[i]);
      }
      return arr;
    }
  
  private:
    std::vector<T> _data;
    std::array<unsigned long int,3> _n_elements;
};

#endif
