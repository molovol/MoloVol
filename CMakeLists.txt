cmake_minimum_required(VERSION 3.20)

# Minimum macOS version, ignored on other platforms
# Must be set before project()
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS deployment version")

# Set name and version
project(MoloVol VERSION 1.0.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# wxWidgets
find_package(wxWidgets REQUIRED gl core base OPTIONAL_COMPONENTS net)
include(${wxWidgets_USE_FILE})

# Add include path, so that header files can be found
include_directories(include)

# Compiler flag for development build
add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>")

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
  src/controller_unittest.cpp
  src/griddata.cpp
  src/misc.cpp
  src/model.cpp
  src/model_filereading.cpp
  src/model_outputfiles.cpp
  src/space.cpp
  src/special_chars.cpp
  src/vector.cpp
  src/voxel.cpp
)

add_executable(MoloVol ${SOURCES})
target_link_libraries(MoloVol PRIVATE ${wxWidgets_LIBRARIES})
set_property(TARGET MoloVol PROPERTY CXX_STANDARD 20)
