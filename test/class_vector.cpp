#include "vector.h"
#include <cassert>

int main(){
  
  { // Default constructor
  
    [[maybe_unused]] const Vector vec;
    assert(vec[0] == 0);
    assert(vec[1] == 0);
    assert(vec[2] == 0);

  }

  { // Construct from three doubles
  
    [[maybe_unused]] const Vector vec(1.2, 2.4, 3.6);
    assert(vec[0] == 1.2);
    assert(vec[1] == 2.4);
    assert(vec[2] == 3.6);

  }

  { // Construct from array
  
    [[maybe_unused]] const Vector vec(std::array<double,3>({4, 5, 6}));
    assert(vec[0] == 4);
    assert(vec[1] == 5);
    assert(vec[2] == 6);

  }

  { // Construct from three ints
    
    [[maybe_unused]] int x, y, z;
    x = 1;
    y = 2;
    z = 3;
  
    [[maybe_unused]] const Vector vec(x, y, z);
    assert(vec[0] == 1);
    assert(vec[1] == 2);
    assert(vec[2] == 3);

  }

  { // Access methods
    Vector vec(1, 2, 3);
    assert(vec[0] == 1);
    assert(vec[1] == 2);
    assert(vec[2] == 3);

    vec.setCoordinate(0,4);
    vec.setCoordinate(1,5);
    vec.setCoordinate(2,6);

    assert(vec.getCoordinate(0) == 4);
    assert(vec.getCoordinate(1) == 5);
    assert(vec.getCoordinate(2) == 6);

    vec[0] = 7;
    vec[1] = 8;
    vec[2] = 9;

    assert(vec.getCoordinate(0) == 7);
    assert(vec.getCoordinate(1) == 8);
    assert(vec.getCoordinate(2) == 9);

    [[maybe_unused]] const Vector c_vec = vec;
    
    assert(c_vec.getCoordinate(0) == c_vec[0]);
    assert(c_vec.getCoordinate(1) == c_vec[1]);
    assert(c_vec.getCoordinate(2) == c_vec[2]);
  }

}
