# Enable testing. This is crucial for CMake to manage and run tests.
enable_testing()

# Create a MoloVol library for the test sources to use
set(TEST_SOURCES
  src/atom.cpp
  src/atomtree.cpp
  src/vector.cpp
  src/importmanager.cpp
  src/crystallographer.cpp
  src/misc.cpp
)

add_library(mvl SHARED ${TEST_SOURCES})
target_include_directories(mvl PUBLIC ./)
target_compile_definitions(mvl PRIVATE LIBRARY_BUILD)

set(TEST_NAMES
  cut_off_string
  struct_atom
  class_vector
  class_atomtree
)

set(MOLOVOL_TEST_DIR ${CMAKE_SOURCE_DIR}/test)
#separate  and explicit to avoid having duplicate main function

# Create separate executables for each unit test
add_executable(vector_test ${MOLOVOL_TEST_DIR}/class_vector.cpp)
add_executable(atom_test ${MOLOVOL_TEST_DIR}/struct_atom.cpp)
add_executable(string_test ${MOLOVOL_TEST_DIR}/cut_off_string.cpp)
add_executable(atomtree_test ${MOLOVOL_TEST_DIR}/class_atomtree.cpp)
add_executable(benchmark_tests ${MOLOVOL_TEST_DIR}/performance_test.cpp)


# Set include directories for all tests
foreach(TEST_TARGET vector_test atom_test string_test atomtree_test benchmark_tests)
    target_link_libraries(${TEST_TARGET} PRIVATE molovol_lib)
    target_include_directories(${TEST_TARGET} PUBLIC 
        ${MOLOVOL_TEST_DIR} 
        ${CMAKE_SOURCE_DIR}/include
    )
endforeach()

# Find the benchmark library. This is required if you have benchmark tests.
find_package(benchmark REQUIRED)
target_link_libraries(benchmark_tests PRIVATE 
    benchmark::benchmark_main
)
add_custom_command(TARGET benchmark_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:benchmark_tests>/inputfile
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/inputfile/elements.txt
        $<TARGET_FILE_DIR:benchmark_tests>/inputfile/
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/inputfile/example_C60.xyz
        $<TARGET_FILE_DIR:benchmark_tests>/inputfile/
    COMMENT "Copying input files for benchmark tests"
)

# Add all tests
add_test(NAME vector_test COMMAND vector_test)
add_test(NAME atom_test COMMAND atom_test)
add_test(NAME string_test COMMAND string_test)
add_test(NAME atomtree_test COMMAND atomtree_test)
add_test(NAME benchmark_tests COMMAND benchmark_tests)
target_compile_definitions(benchmark_tests PRIVATE 
    SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    foreach(TEST_TARGET vector_test atom_test string_test atomtree_test benchmark_tests)
        target_compile_options(${TEST_TARGET} PRIVATE -Wall -Wextra -Werror)
    endforeach()
endif()

add_custom_target(build_tests ALL 
    DEPENDS vector_test atom_test string_test atomtree_test benchmark_tests)
	