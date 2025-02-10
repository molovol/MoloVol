# Enable testing. This is crucial for CMake to manage and run tests.
enable_testing()

# First, create the unit test executable
set(MOLOVOL_TEST_DIR ${CMAKE_SOURCE_DIR}/test)
#separate  and explicit to avoid having duplicate main function

# Create separate executables for each unit test
add_executable(vector_test ${MOLOVOL_TEST_DIR}/class_vector.cpp)
add_executable(atom_test ${MOLOVOL_TEST_DIR}/struct_atom.cpp)
add_executable(string_test ${MOLOVOL_TEST_DIR}/cut_off_string.cpp)
add_executable(benchmark_tests ${MOLOVOL_TEST_DIR}/performance_test.cpp)


# Set include directories for all tests
foreach(TEST_TARGET vector_test atom_test string_test benchmark_tests)
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

# Add all tests
add_test(NAME vector_test COMMAND vector_test)
add_test(NAME atom_test COMMAND atom_test)
add_test(NAME string_test COMMAND string_test)
add_test(NAME benchmark_tests COMMAND benchmark_tests)
target_compile_definitions(benchmark_tests PRIVATE 
    SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    foreach(TEST_TARGET vector_test atom_test string_test benchmark_tests)
        target_compile_options(${TEST_TARGET} PRIVATE -Wall -Wextra -Werror)
    endforeach()
endif()

add_custom_target(build_tests ALL 
    DEPENDS vector_test atom_test string_test benchmark_tests)
	
