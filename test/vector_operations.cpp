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

}
