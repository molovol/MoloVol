#include <benchmark/benchmark.h>
#include "vector.h"
#include "atom.h"
#include "space.h"
#include "model.h"
#include "importmanager.h"
#include <vector>
#include <string>
#include <cmath>

// Performance test for Vector class operations
static void BM_VectorOperations(benchmark::State& state) {
  Vector vec1(1.2, 2.4, 3.6);
  Vector vec2(6.0, 4.8, 3.0);

  for (auto _ : state) {
    benchmark::DoNotOptimize(vec1 + vec2);
    benchmark::DoNotOptimize(vec1 - vec2);
    benchmark::DoNotOptimize(vec1 * 2.0);
    benchmark::DoNotOptimize(vec1.length());
    benchmark::DoNotOptimize(vec1 * vec2);
    benchmark::DoNotOptimize(distance(vec1, vec2));
  }
}
BENCHMARK(BM_VectorOperations);

// Performance test for Atom class operations
static void BM_AtomOperations(benchmark::State& state) {
  // Using the 6-argument constructor from atom.h
  Atom atom1(1.0, 2.0, 3.0, "C", 1.7, 6);
  Atom atom2(4.0, 5.0, 6.0, "O", 1.52, 8);

  for (auto _ : state) {
    benchmark::DoNotOptimize(distance(atom1.getPosVec(), atom2.getPosVec()));
    benchmark::DoNotOptimize(atom1.getRad() + atom2.getRad());
    benchmark::DoNotOptimize(atom1.number + atom2.number);
  }
}
BENCHMARK(BM_AtomOperations);

// Performance test for Space class operations
static void BM_SpaceOperations(benchmark::State& state) {
  std::vector<Atom> atoms = {
    Atom(1.0, 2.0, 3.0, "C", 1.7, 6),
    Atom(4.0, 5.0, 6.0, "O", 1.52, 8),
    Atom(7.0, 8.0, 9.0, "H", 1.2, 1)
  };

  double grid_size = 0.2;
  int depth = 4;
  double r_probe = 1.4;
  bool unit_cell_option = false;
  std::array<double, 3> unit_cell_axes = {0, 0, 0};

  Space space(atoms, grid_size, depth, r_probe, unit_cell_option, unit_cell_axes);

  for (auto _ : state) {
    std::vector<Cavity> cavities;  // Create a vector to pass by reference
    bool output_flag = false;  // Added missing boolean reference parameter
    space.assignTypeInGrid(atoms, cavities, 1.4, 0, false, output_flag);
  }
}
BENCHMARK(BM_SpaceOperations);

// Performance test for Model class operations
static void BM_ModelOperations(benchmark::State& state) {
  std::string elements_file_path = "inputfile/elements.txt";
  std::string structure_file_path = "inputfile/example_C60.xyz";

  Model model;
  try {
    if (!model.importElemFile(elements_file_path)) {
      state.SkipWithError("Failed to import elements file");
      return;
    }
    if (!model.readAtomsFromFile(structure_file_path, false)) {
      state.SkipWithError("Failed to read structure file");
      return;
    }

    for (auto _ : state) {
      model.generateData();
    }
  } catch (const std::exception& e) {
    state.SkipWithError(e.what());
  }
}
BENCHMARK(BM_ModelOperations);

// Performance test for Model::processUnitCell
static void BM_ModelProcessUnitCell(benchmark::State& state) {
  std::string structure_file_path = "inputfile/example_C60.xyz";
  std::string elements_file_path = "inputfile/elements.txt";

  Model model;
  model.importElemFile(elements_file_path);
  model.readAtomsFromFile(structure_file_path, false);  // Added missing boolean parameter

  for (auto _ : state) {
    model.processUnitCell();
  }
}
BENCHMARK(BM_ModelProcessUnitCell);