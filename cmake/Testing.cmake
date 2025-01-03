
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

foreach(TN IN ITEMS ${TEST_NAMES})

  set(TEST_SRC_NAME ${TN}.cpp)
  set(TEST_EXE_NAME t_${TN})
  add_executable(${TEST_EXE_NAME} ${MOLOVOL_TEST_DIR}/${TEST_SRC_NAME})
  set_target_properties(${TEST_EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testbin)
  target_include_directories(${TEST_EXE_NAME} PUBLIC ${MOLOVOL_TEST_DIR})
  target_link_libraries(${TEST_EXE_NAME} mvl)
  add_test(NAME ${TN} COMMAND ${TEST_EXE_NAME})

endforeach()

