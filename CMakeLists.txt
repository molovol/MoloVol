cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Options)
# MOLOVOL_ABS_RESOURCE_PATH
# MOLOVOL_OSX_FAT_FILE
# MOLOVOL_BUILD_TESTING

# Strip binary for release build
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

# Minimum macOS version, ignored on other platforms
# Must be set before project()
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS deployment version")

# Set name and version
project(MoloVol VERSION 1.1.0)

include(ExecutableName)
# EXE_NAME

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Make universal binary
if(MOLOVOL_OSX_FAT_FILE)
  set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
endif()

# wxWidgets
set(wxWidgets_USE_STATIC=ON)
find_package(wxWidgets REQUIRED core base OPTIONAL_COMPONENTS net)
include(${wxWidgets_USE_FILE})

#find_package(OpenMP)

# Add include path, so that header files can be found
include_directories(include)

# Compiler flag for development build
add_compile_options("$<$<NOT:$<CONFIG:RELEASE,MINSIZEREL,RELWITHDEBINFO>>:-DDEBUG>")
add_compile_options(-Wall -Werror -Wno-unused-command-line-argument -Wno-invalid-source-encoding)

include(Sources)
# SOURCES

include(Resources)
# ELEM_FILE
# SPACEGROUP_FILE
# OSX_RESOURCE_FILES

if(NOT MOLOVOL_ABS_RESOURCE_PATH)
  add_custom_target(copy-resource-files ALL
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --blue "Copying resource files to build directory"
    COMMAND ${CMAKE_COMMAND} -E copy
      ${ELEM_FILE}
      ${CMAKE_CURRENT_BINARY_DIR}/inputfile/elements.txt
    COMMAND ${CMAKE_COMMAND} -E copy
      ${SPACEGROUP_FILE}
      ${CMAKE_CURRENT_BINARY_DIR}/inputfile/space_groups.txt
  )
endif()

# Target MoloVol
add_executable(${EXE_NAME} ${SOURCES} ${OSX_RESOURCE_FILES})

# XCode, app bundle and libtiff
include(MacSpecific)

target_link_libraries(${EXE_NAME} ${wxWidgets_LIBRARIES})
  # Add custom flag
if(MOLOVOL_ABS_RESOURCE_PATH)
  target_compile_definitions(${EXE_NAME} PUBLIC -DABS_PATH)
endif()

# Keeping this around just so I don't forget the syntax
#  if(OpenMP_CXX_FOUND)
#    target_link_libraries(target PUBLIC OpenMP::OpenMP_CXX)
#  endif()

# Tests
if (MOLOVOL_BUILD_TESTING AND BUILD_TESTING)
  include(Testing)
endif()

# Installation instructions for debian package
if (UNIX AND NOT APPLE)
  include(DebInstall)
elseif (APPLE)
  include(MacInstall)
endif()

include(Packing)

