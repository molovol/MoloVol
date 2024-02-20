#include "atom.h"
#include <utility>
#include <iostream>
#include <chrono>

bool validateAtom(const Atom&, Atom::num_type, Atom::num_type, Atom::num_type, 
    Atom::symbol_type, Atom::num_type, Atom::atomic_num_type, Atom::charge_type);

int main(int argc, char* argv[]){
  // Default constructor
  Atom at;

  if (!validateAtom(at, 0, 0, 0, "", -1, 0, 0)) return -1;
  if (at.isValid()) return -1; // Default constructed atom is always invalid
  at.rad = 1; 
  if (!at.isValid()) return -1;

  // Construct with position and symbol
  at = Atom(std::make_pair("Gh", Atom::pos_type({1, 2, 3})));
  if (!validateAtom(at, 1, 2, 3, "Gh", -1, 0, 0)) return -1;

  // Construct with position, symbol and charge
  at = Atom(std::make_pair("Gh", Atom::pos_type({1, 2, 3})), -2);
  if (!validateAtom(at, 1, 2, 3, "Gh", -1, 0, -2)) return -1;

  // Construct with position, symbol, radius and number
  at = Atom(1, 2, 3, "Gh", 1.2, 34);
  if (!validateAtom(at, 1, 2, 3, "Gh", 1.2, 34, 0)) return -1;

  // Construct with position, symbol, radius, number and charge
  at = Atom(1, 2, 3, "Gh", 1.2, 34, -2);
  if (!validateAtom(at, 1, 2, 3, "Gh", 1.2, 34, -2)) return -1;
  if (!at.isValid()) return -1;

  // Comparison operator
  if (at != at) return -1;

  // Benchmark
  if (argc > 1 && !strcmp(argv[1], "benchmark")){

    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
 
    int n_test_cycle = 1000000;

    auto pp = std::make_pair("Gh", Atom::pos_type({1,2,3}));
    begin = std::chrono::steady_clock::now();
    for (int i = 1; i < n_test_cycle; ++i){
      at = Atom(pp, -2);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "Array-based constructor: " 
      << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    begin = std::chrono::steady_clock::now();
    for (int i = 1; i < n_test_cycle; ++i){
      at = Atom(1, 2, 3, "Gh", 1.2, 34, -2);
    }
    end = std::chrono::steady_clock::now();
    std::cout << "Number-based constructor: " 
      << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
  }
}

bool validateAtom(
    const Atom& at,
    Atom::num_type x,
    Atom::num_type y,
    Atom::num_type z,
    Atom::symbol_type sym,
    Atom::num_type radius,
    Atom::atomic_num_type at_num,
    Atom::charge_type charge){
  
  bool valid = true;

  valid &= (at.pos_x == x);
  valid &= (at.pos_y == y);
  valid &= (at.pos_z == z);
  valid &= (at.symbol == sym);
  valid &= (at.rad == radius);
  valid &= (at.charge == charge);
  valid &= (at.number == at_num);

  [[maybe_unused]] auto pos_array = at.getPos();
  valid &= (pos_array[0] == x);
  valid &= (pos_array[1] == y);
  valid &= (pos_array[2] == z);

  [[maybe_unused]] auto pos_vec = at.getPosVec();
  valid &= (pos_vec[0] == x);
  valid &= (pos_vec[1] == y);
  valid &= (pos_vec[2] == z);

  valid &= (at.getCoordinate(0) == x);
  valid &= (at.getCoordinate(1) == y);
  valid &= (at.getCoordinate(2) == z);

  valid &= (at.getRad() == radius);

  return valid;
}
