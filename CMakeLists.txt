cmake_minimum_required(VERSION 3.20)

# Custom options
option(ABS_RESOURCE_PATH "Set a platform-dependant, absolute path to the resource directory" OFF)
option(OSX_FAT_FILE "Make executable a fat file with architectures x86_64 and arm64" OFF)
option(ENABLE_TEST "Add target 'test' to allow quick compilation of test/main.cpp" OFF)

# Minimum macOS version, ignored on other platforms
# Must be set before project()
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS deployment version")

# Set name and version
project(MoloVol VERSION 1.0.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Make universal binary
if(OSX_FAT_FILE)
  set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
endif()

# wxWidgets
set(wxWidgets_USE_STATIC=ON)
find_package(wxWidgets REQUIRED core base OPTIONAL_COMPONENTS net)
include(${wxWidgets_USE_FILE})

find_package(OpenMP)

# Add include path, so that header files can be found
include_directories(include)

# Compiler flag for development build
add_compile_options("$<$<NOT:$<CONFIG:RELEASE,MINSIZEREL,RELWITHDEBINFO>>:-DDEBUG>")
add_compile_options(-Wall -Werror -Wno-unused-command-line-argument -Wno-invalid-source-encoding)

# List of source files
set(SOURCES
  src/atomtree.cpp
  src/base_guicontrol.cpp
  src/base_cmdline.cpp
  src/base_constr.cpp
  src/base_event.cpp
  src/base_guicontrol.cpp
  src/base_init.cpp
  src/cavity.cpp
  src/controller.cpp
  src/griddata.cpp
  src/importmanager.cpp
  src/misc.cpp
  src/model.cpp
  src/model_filereading.cpp
  src/model_outputfiles.cpp
  src/space.cpp
  src/special_chars.cpp
  src/vector.cpp
  src/voxel.cpp
)

# Resource files
if(NOT ABS_RESOURCE_PATH)
  add_custom_target(copy-resource-files ALL
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --blue "Copying resource files to build directory"
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/inputfile/elements.txt
      ${CMAKE_CURRENT_BINARY_DIR}/inputfile/elements.txt
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/inputfile/space_groups.txt
      ${CMAKE_CURRENT_BINARY_DIR}/inputfile/space_groups.txt
  )
endif()

# Target MoloVol
add_executable(MoloVol ${SOURCES})
  # XCode compatibility
set_target_properties(MoloVol PROPERTIES
  XCODE_GENERATE_SCHEME TRUE
  XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
  # Library shenanigans
string(REPLACE "-ltiff" "/usr/local/opt/libtiff/lib/libtiff.a" wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
target_link_libraries(MoloVol ${wxWidgets_LIBRARIES})
  # Add custom flag
if(ABS_RESOURCE_PATH)
  target_compile_definitions(MoloVol PUBLIC -DABS_PATH)
endif()

# Target test
if(ENABLE_TEST)
  set(TEST_SOURCES
    test/main.cpp
  )
  
  add_executable(test EXCLUDE_FROM_ALL ${TEST_SOURCES})
  if(OpenMP_CXX_FOUND)
    target_link_libraries(test PUBLIC OpenMP::OpenMP_CXX)
  endif()
endif()

