#include "vector.h"
#include <cmath>

bool validateCoordinates(Vector, double, double, double);

int main(){
  
  { // Default constructor
  
    [[maybe_unused]] const Vector vec;
    if (!validateCoordinates(vec, 0, 0, 0)) return -1;

  }

  { // Construct from three doubles
  
    [[maybe_unused]] const Vector vec(1.2, 2.4, 3.6);
    if (!validateCoordinates(vec, 1.2, 2.4, 3.6)) return -1;

  }

  { // Construct from array
  
    [[maybe_unused]] const Vector vec(std::array<double,3>({4, 5, 6}));
    if (!validateCoordinates(vec, 4, 5, 6)) return -1;

  }

  { // Construct from three ints
    
    [[maybe_unused]] int x, y, z;
    x = 1;
    y = 2;
    z = 3;
  
    [[maybe_unused]] const Vector vec(x, y, z);
    if (!validateCoordinates(vec, 1, 2, 3)) return -1;

  }

  { // Access methods
    Vector vec(1, 2, 3);
    if (!validateCoordinates(vec, 1, 2, 3)) return -1;

    vec.setCoordinate(0,4);
    vec.setCoordinate(1,5);
    vec.setCoordinate(2,6);

    if (!validateCoordinates(vec, 4, 5, 6)) return -1;

    vec[0] = 7;
    vec[1] = 8;
    vec[2] = 9;

    if (!validateCoordinates(vec, 7, 8, 9)) return -1;

    [[maybe_unused]] const Vector c_vec = vec;
    
    if (!validateCoordinates(vec, c_vec[0], c_vec[1], c_vec[2])) return -1;
  }

  { // Mathematical operations
    
    Vector lhs(1, 3, 5);
    Vector rhs(6, 4, 2);
    const Vector c_lhs = lhs;
    const Vector c_rhs = rhs;

    { // Addition and subtraction
      [[maybe_unused]] Vector sum = lhs + rhs;
      if (!validateCoordinates(sum, 7, 7, 7)) return -1;
      sum = add(lhs, rhs);
      if (!validateCoordinates(sum, 7, 7, 7)) return -1;

      [[maybe_unused]] Vector diff = lhs - rhs;
      if (!validateCoordinates(diff, -5, -1, 3)) return -1;

      // const
      if (!validateCoordinates(c_lhs + c_rhs, 7, 7, 7)) return -1;
      if (!validateCoordinates(add(c_lhs, c_rhs), 7, 7, 7)) return -1;

      [[maybe_unused]] Vector c_diff = c_lhs - c_rhs;
      if (!validateCoordinates(c_diff, -5, -1, 3)) return -1;
    }

    { // Multiplication
      if (!validateCoordinates(scale(lhs, 3), 3, 9, 15)) return -1;
      if (!validateCoordinates(scale(3, lhs), 3, 9, 15)) return -1;

      if (!validateCoordinates(lhs * 5, 5, 15, 25)) return -1;
      if (!validateCoordinates(5 * lhs, 5, 15, 25)) return -1;

      if (!validateCoordinates(lhs / 2, 0.5, 1.5, 2.5)) return -1;

      if (!(dotproduct(lhs, rhs) == 28)) return -1;
      if (!(lhs * rhs == 28)) return -1;
      
      // const
      if (!validateCoordinates(scale(c_lhs, 3), 3, 9, 15)) return -1;
      if (!validateCoordinates(scale(3, c_lhs), 3, 9, 15)) return -1;

      if (!validateCoordinates(c_lhs * 5, 5, 15, 25)) return -1;
      if (!validateCoordinates(5 * c_lhs, 5, 15, 25)) return -1;

      if (!validateCoordinates(c_lhs / 2, 0.5, 1.5, 2.5)) return -1;

      if (!(dotproduct(c_lhs, c_rhs) == 28)) return -1;
      if (!(c_lhs * c_rhs == 28)) return -1;
    }

    { // Other
      if (!(distance(lhs, rhs) == sqrt(35))) return -1;

      if (!(distance(lhs, rhs, 0) == 5)) return -1;
      if (!(distance(lhs, rhs, 1) == 1)) return -1;
      if (!(distance(lhs, rhs, 2) == -3)) return -1;

      if (!(lhs.length() == sqrt(35))) return -1;

      if (!validateCoordinates(Vector(4,4,7).normalise(), 4.0/9.0, 4.0/9.0, 7.0/9.0)) return -1;

      // const
      if (!(distance(c_lhs, c_rhs) == sqrt(35))) return -1;

      if (!(distance(c_lhs, c_rhs, 0) == 5)) return -1;
      if (!(distance(c_lhs, c_rhs, 1) == 1)) return -1;
      if (!(distance(c_lhs, c_rhs, 2) == -3)) return -1;

      if (!(c_lhs.length() == sqrt(35))) return -1;

      const Vector vec(4,4,7);
      if (!validateCoordinates(vec.normalise(), 4.0/9.0, 4.0/9.0, 7.0/9.0)) return -1;
    }
  }

  { // Comparison operators
    Vector vec(3,6,2);
    Vector longer(7,9,9);
    Vector shorter(1,2,1);
    Vector equal(2,3,6);

    if (!(!vec.isLongerThan(longer))) return -1;
    if (!( vec.isLongerThan(shorter))) return -1;
    if (!(!vec.isLongerThan(equal))) return -1;
    if (!(!vec.isLongerThan(longer.length()))) return -1;
    if (!( vec.isLongerThan(shorter.length()))) return -1;
    if (!(!vec.isLongerThan(equal.length()))) return -1;

    if (!(!(vec > longer))) return -1;
    if (!( (vec > shorter))) return -1;
    if (!(!(vec > equal))) return -1;
    if (!(!(vec > longer.length()))) return -1;
    if (!( (vec > shorter.length()))) return -1;
    if (!(!(vec > equal.length()))) return -1;

    if (!(!(vec >= longer))) return -1;
    if (!( (vec >= shorter))) return -1;
    if (!( (vec >= equal))) return -1;
    if (!(!(vec >= longer.length()))) return -1;
    if (!( (vec >= shorter.length()))) return -1;
    if (!( (vec >= equal.length()))) return -1;

    if (!( vec.isShorterThan(longer))) return -1;
    if (!(!vec.isShorterThan(shorter))) return -1;
    if (!(!vec.isShorterThan(equal))) return -1;
    if (!( vec.isShorterThan(longer.length()))) return -1;
    if (!(!vec.isShorterThan(shorter.length()))) return -1;
    if (!(!vec.isShorterThan(equal.length()))) return -1;

    if (!( (vec < longer))) return -1;
    if (!(!(vec < shorter))) return -1;
    if (!(!(vec < equal))) return -1;
    if (!( (vec < longer.length()))) return -1;
    if (!(!(vec < shorter.length()))) return -1;
    if (!(!(vec < equal.length()))) return -1;
    
    if (!( (vec <= longer))) return -1;
    if (!(!(vec <= shorter))) return -1;
    if (!( (vec <= equal))) return -1;
    if (!( (vec <= longer.length()))) return -1;
    if (!(!(vec <= shorter.length()))) return -1;
    if (!( (vec <= equal.length()))) return -1;

    if (!(!vec.isSameLength(longer))) return -1;
    if (!(!vec.isSameLength(shorter))) return -1;
    if (!( vec.isSameLength(equal))) return -1;
    if (!(!vec.isSameLength(longer.length()))) return -1;
    if (!(!vec.isSameLength(shorter.length()))) return -1;
    if (!( vec.isSameLength(equal.length()))) return -1;

    if (!(!(vec == longer))) return -1;
    if (!(!(vec == shorter))) return -1;
    if (!( (vec == equal))) return -1;
    if (!(!(vec == longer.length()))) return -1;
    if (!(!(vec == shorter.length()))) return -1;
    if (!( (vec == equal.length()))) return -1;

    if (!( (vec != longer))) return -1;
    if (!( (vec != shorter))) return -1;
    if (!(!(vec != equal))) return -1;
    if (!( (vec != longer.length()))) return -1;
    if (!( (vec != shorter.length()))) return -1;
    if (!(!(vec != equal.length()))) return -1;
  }

  { // Comparison operators const
    const Vector vec(3,6,2);
    const Vector longer(7,9,9);
    const Vector shorter(1,2,1);
    const Vector equal(2,3,6);

    if (!(!vec.isLongerThan(longer))) return -1;
    if (!( vec.isLongerThan(shorter))) return -1;
    if (!(!vec.isLongerThan(equal))) return -1;
    if (!(!vec.isLongerThan(longer.length()))) return -1;
    if (!( vec.isLongerThan(shorter.length()))) return -1;
    if (!(!vec.isLongerThan(equal.length()))) return -1;

    if (!(!(vec > longer))) return -1;
    if (!( (vec > shorter))) return -1;
    if (!(!(vec > equal))) return -1;
    if (!(!(vec > longer.length()))) return -1;
    if (!( (vec > shorter.length()))) return -1;
    if (!(!(vec > equal.length()))) return -1;

    if (!(!(vec >= longer))) return -1;
    if (!( (vec >= shorter))) return -1;
    if (!( (vec >= equal))) return -1;
    if (!(!(vec >= longer.length()))) return -1;
    if (!( (vec >= shorter.length()))) return -1;
    if (!( (vec >= equal.length()))) return -1;

    if (!( vec.isShorterThan(longer))) return -1;
    if (!(!vec.isShorterThan(shorter))) return -1;
    if (!(!vec.isShorterThan(equal))) return -1;
    if (!( vec.isShorterThan(longer.length()))) return -1;
    if (!(!vec.isShorterThan(shorter.length()))) return -1;
    if (!(!vec.isShorterThan(equal.length()))) return -1;

    if (!( (vec < longer))) return -1;
    if (!(!(vec < shorter))) return -1;
    if (!(!(vec < equal))) return -1;
    if (!( (vec < longer.length()))) return -1;
    if (!(!(vec < shorter.length()))) return -1;
    if (!(!(vec < equal.length()))) return -1;
    
    if (!( (vec <= longer))) return -1;
    if (!(!(vec <= shorter))) return -1;
    if (!( (vec <= equal))) return -1;
    if (!( (vec <= longer.length()))) return -1;
    if (!(!(vec <= shorter.length()))) return -1;
    if (!( (vec <= equal.length()))) return -1;

    if (!(!vec.isSameLength(longer))) return -1;
    if (!(!vec.isSameLength(shorter))) return -1;
    if (!( vec.isSameLength(equal))) return -1;
    if (!(!vec.isSameLength(longer.length()))) return -1;
    if (!(!vec.isSameLength(shorter.length()))) return -1;
    if (!( vec.isSameLength(equal.length()))) return -1;

    if (!(!(vec == longer))) return -1;
    if (!(!(vec == shorter))) return -1;
    if (!( (vec == equal))) return -1;
    if (!(!(vec == longer.length()))) return -1;
    if (!(!(vec == shorter.length()))) return -1;
    if (!( (vec == equal.length()))) return -1;

    if (!( (vec != longer))) return -1;
    if (!( (vec != shorter))) return -1;
    if (!(!(vec != equal))) return -1;
    if (!( (vec != longer.length()))) return -1;
    if (!( (vec != shorter.length()))) return -1;
    if (!(!(vec != equal.length()))) return -1;
  }

}

bool validateCoordinates(Vector vec, double x, double y, double z){
  [[maybe_unused]] double small = 0.0000001;
  return 
    abs(vec.getCoordinate(0)-x) < small
    && abs(vec.getCoordinate(1)-y) < small
    && abs(vec.getCoordinate(2)-z) < small;
}

