
enable_testing()
# Create a MoloVol library for the test sources to use
set(TEST_SOURCES
  src/importmanager.cpp
  src/crystallographer.cpp
  src/misc.cpp
)
add_library(mvl SHARED ${TEST_SOURCES})
target_include_directories(mvl PUBLIC ./)
target_compile_definitions(mvl PRIVATE LIBRARY_BUILD)

set(TEST_NAME cut_off_string)
set(TEST_SRC_NAME ${TEST_NAME}.cpp)
set(TEST_EXE_NAME t_${TEST_NAME})
add_executable(${TEST_EXE_NAME} test/${TEST_SRC_NAME})
target_include_directories(${TEST_EXE_NAME} PUBLIC ./test)
target_link_libraries(${TEST_EXE_NAME} mvl)
add_test(NAME "Should cut off string after non-letter" COMMAND ${TEST_EXE_NAME})

