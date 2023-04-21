#include "vector.h"
#include <cassert>
#include <cmath>

void validateCoordinates(Vector, double, double, double);

int main(){
  
  { // Default constructor
  
    [[maybe_unused]] const Vector vec;
    validateCoordinates(vec, 0, 0, 0);

  }

  { // Construct from three doubles
  
    [[maybe_unused]] const Vector vec(1.2, 2.4, 3.6);
    validateCoordinates(vec, 1.2, 2.4, 3.6);

  }

  { // Construct from array
  
    [[maybe_unused]] const Vector vec(std::array<double,3>({4, 5, 6}));
    validateCoordinates(vec, 4, 5, 6);

  }

  { // Construct from three ints
    
    [[maybe_unused]] int x, y, z;
    x = 1;
    y = 2;
    z = 3;
  
    [[maybe_unused]] const Vector vec(x, y, z);
    validateCoordinates(vec, 1, 2, 3);

  }

  { // Access methods
    Vector vec(1, 2, 3);
    validateCoordinates(vec, 1, 2, 3);

    vec.setCoordinate(0,4);
    vec.setCoordinate(1,5);
    vec.setCoordinate(2,6);

    validateCoordinates(vec, 4, 5, 6);

    vec[0] = 7;
    vec[1] = 8;
    vec[2] = 9;

    validateCoordinates(vec, 7, 8, 9);

    [[maybe_unused]] const Vector c_vec = vec;
    
    validateCoordinates(vec, c_vec[0], c_vec[1], c_vec[2]);
  }

  { // Mathematical operations
    
    Vector lhs(1, 3, 5);
    Vector rhs(6, 4, 2);
    const Vector c_lhs = lhs;
    const Vector c_rhs = rhs;

    { // Addition and subtraction
      [[maybe_unused]] Vector sum = lhs + rhs;
      validateCoordinates(sum, 7, 7, 7);
      sum = add(lhs, rhs);
      validateCoordinates(sum, 7, 7, 7);

      [[maybe_unused]] Vector diff = lhs - rhs;
      validateCoordinates(diff, -5, -1, 3);

      // const
      validateCoordinates(c_lhs + c_rhs, 7, 7, 7);
      validateCoordinates(add(c_lhs, c_rhs), 7, 7, 7);

      [[maybe_unused]] Vector c_diff = c_lhs - c_rhs;
      validateCoordinates(c_diff, -5, -1, 3);
    }

    { // Multiplication
      validateCoordinates(scale(lhs, 3), 3, 9, 15);
      validateCoordinates(scale(3, lhs), 3, 9, 15);

      validateCoordinates(lhs * 5, 5, 15, 25);
      validateCoordinates(5 * lhs, 5, 15, 25);

      validateCoordinates(lhs / 2, 0.5, 1.5, 2.5);

      assert(dotproduct(lhs, rhs) == 28);
      assert(lhs * rhs == 28);
      
      // const
      validateCoordinates(scale(c_lhs, 3), 3, 9, 15);
      validateCoordinates(scale(3, c_lhs), 3, 9, 15);

      validateCoordinates(c_lhs * 5, 5, 15, 25);
      validateCoordinates(5 * c_lhs, 5, 15, 25);

      validateCoordinates(c_lhs / 2, 0.5, 1.5, 2.5);

      assert(dotproduct(c_lhs, c_rhs) == 28);
      assert(c_lhs * c_rhs == 28);
    }

    { // Other
      assert(distance(lhs, rhs) == sqrt(35));

      assert(distance(lhs, rhs, 0) == 5);
      assert(distance(lhs, rhs, 1) == 1);
      assert(distance(lhs, rhs, 2) == -3);

      assert(lhs.length() == sqrt(35));

      validateCoordinates(Vector(4,4,7).normalise(), 4.0/9.0, 4.0/9.0, 7.0/9.0);

      // const
      assert(distance(c_lhs, c_rhs) == sqrt(35));

      assert(distance(c_lhs, c_rhs, 0) == 5);
      assert(distance(c_lhs, c_rhs, 1) == 1);
      assert(distance(c_lhs, c_rhs, 2) == -3);

      assert(c_lhs.length() == sqrt(35));

      const Vector vec(4,4,7);
      validateCoordinates(vec.normalise(), 4.0/9.0, 4.0/9.0, 7.0/9.0);
    }
  }

  { // Comparison operators
    Vector vec(3,6,2);
    Vector longer(7,9,9);
    Vector shorter(1,2,1);
    Vector equal(2,3,6);

    assert(!vec.isLongerThan(longer));
    assert( vec.isLongerThan(shorter));
    assert(!vec.isLongerThan(equal));
    assert(!vec.isLongerThan(longer.length()));
    assert( vec.isLongerThan(shorter.length()));
    assert(!vec.isLongerThan(equal.length()));

    assert(!(vec > longer));
    assert( (vec > shorter));
    assert(!(vec > equal));
    assert(!(vec > longer.length()));
    assert( (vec > shorter.length()));
    assert(!(vec > equal.length()));

    assert(!(vec >= longer));
    assert( (vec >= shorter));
    assert( (vec >= equal));
    assert(!(vec >= longer.length()));
    assert( (vec >= shorter.length()));
    assert( (vec >= equal.length()));

    assert( vec.isShorterThan(longer));
    assert(!vec.isShorterThan(shorter));
    assert(!vec.isShorterThan(equal));
    assert( vec.isShorterThan(longer.length()));
    assert(!vec.isShorterThan(shorter.length()));
    assert(!vec.isShorterThan(equal.length()));

    assert( (vec < longer));
    assert(!(vec < shorter));
    assert(!(vec < equal));
    assert( (vec < longer.length()));
    assert(!(vec < shorter.length()));
    assert(!(vec < equal.length()));
    
    assert( (vec <= longer));
    assert(!(vec <= shorter));
    assert( (vec <= equal));
    assert( (vec <= longer.length()));
    assert(!(vec <= shorter.length()));
    assert( (vec <= equal.length()));

    assert(!vec.isSameLength(longer));
    assert(!vec.isSameLength(shorter));
    assert( vec.isSameLength(equal));
    assert(!vec.isSameLength(longer.length()));
    assert(!vec.isSameLength(shorter.length()));
    assert( vec.isSameLength(equal.length()));

    assert(!(vec == longer));
    assert(!(vec == shorter));
    assert( (vec == equal));
    assert(!(vec == longer.length()));
    assert(!(vec == shorter.length()));
    assert( (vec == equal.length()));

    assert( (vec != longer));
    assert( (vec != shorter));
    assert(!(vec != equal));
    assert( (vec != longer.length()));
    assert( (vec != shorter.length()));
    assert(!(vec != equal.length()));
  }

  { // Comparison operators const
    const Vector vec(3,6,2);
    const Vector longer(7,9,9);
    const Vector shorter(1,2,1);
    const Vector equal(2,3,6);

    assert(!vec.isLongerThan(longer));
    assert( vec.isLongerThan(shorter));
    assert(!vec.isLongerThan(equal));
    assert(!vec.isLongerThan(longer.length()));
    assert( vec.isLongerThan(shorter.length()));
    assert(!vec.isLongerThan(equal.length()));

    assert(!(vec > longer));
    assert( (vec > shorter));
    assert(!(vec > equal));
    assert(!(vec > longer.length()));
    assert( (vec > shorter.length()));
    assert(!(vec > equal.length()));

    assert(!(vec >= longer));
    assert( (vec >= shorter));
    assert( (vec >= equal));
    assert(!(vec >= longer.length()));
    assert( (vec >= shorter.length()));
    assert( (vec >= equal.length()));

    assert( vec.isShorterThan(longer));
    assert(!vec.isShorterThan(shorter));
    assert(!vec.isShorterThan(equal));
    assert( vec.isShorterThan(longer.length()));
    assert(!vec.isShorterThan(shorter.length()));
    assert(!vec.isShorterThan(equal.length()));

    assert( (vec < longer));
    assert(!(vec < shorter));
    assert(!(vec < equal));
    assert( (vec < longer.length()));
    assert(!(vec < shorter.length()));
    assert(!(vec < equal.length()));
    
    assert( (vec <= longer));
    assert(!(vec <= shorter));
    assert( (vec <= equal));
    assert( (vec <= longer.length()));
    assert(!(vec <= shorter.length()));
    assert( (vec <= equal.length()));

    assert(!vec.isSameLength(longer));
    assert(!vec.isSameLength(shorter));
    assert( vec.isSameLength(equal));
    assert(!vec.isSameLength(longer.length()));
    assert(!vec.isSameLength(shorter.length()));
    assert( vec.isSameLength(equal.length()));

    assert(!(vec == longer));
    assert(!(vec == shorter));
    assert( (vec == equal));
    assert(!(vec == longer.length()));
    assert(!(vec == shorter.length()));
    assert( (vec == equal.length()));

    assert( (vec != longer));
    assert( (vec != shorter));
    assert(!(vec != equal));
    assert( (vec != longer.length()));
    assert( (vec != shorter.length()));
    assert(!(vec != equal.length()));
  }

}

void validateCoordinates(Vector vec, double x, double y, double z){
  [[maybe_unused]] double small = 0.0000001;
  assert(abs(vec.getCoordinate(0)-x) < small);
  assert(abs(vec.getCoordinate(1)-y) < small);
  assert(abs(vec.getCoordinate(2)-z) < small);
}

