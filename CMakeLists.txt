cmake_minimum_required(VERSION 3.20)

# Custom options
option(ABS_RESOURCE_PATH "Set a platform-dependant, absolute path to the resource directory" OFF)
option(OSX_FAT_FILE "Make executable a fat file with architectures x86_64 and arm64" OFF)
option(MOLOVOL_BUILD_TESTING "Enables unit tests so that test source files are compiled along with executable" OFF)

if (CMAKE_MACOSX_BUNDLE AND NOT ABS_RESOURCE_PATH)
  message(WARNING "The executable inside a macOS application bundle is always compiled with ABS_RESOURCE_PATH enabled")
  set(ABS_RESOURCE_PATH TRUE)
endif()

# Minimum macOS version, ignored on other platforms
# Must be set before project()
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS deployment version")

# Set name and version
project(MoloVol VERSION 1.1.0)

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
  src/crystallographer.cpp
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
set(ELEM_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inputfile/elements.txt")
set(SPACEGROUP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inputfile/space_groups.txt")

if(NOT ABS_RESOURCE_PATH)
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

# Resource files for macOS Bundle
set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/macOS/icon.icns")
set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties(${ELEM_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties(${SPACEGROUP_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set(RESOURCE_FILES ${ICON_FILE} ${ELEM_FILE} ${SPACEGROUP_FILE})

# Target MoloVol
add_executable(MoloVol ${SOURCES} ${RESOURCE_FILES})
  # XCode compatibility
set_target_properties(MoloVol PROPERTIES
  XCODE_GENERATE_SCHEME TRUE
  XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
  # macOS Bundle
set_target_properties(MoloVol PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME MoloVol
  MACOSX_BUNDLE_EXECUTABLE_NAME MoloVol
  MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
  MACOSX_BUNDLE_ICON_FILE icon
)

  # Library shenanigans
string(REPLACE "-ltiff" "/usr/local/opt/libtiff/lib/libtiff.a" wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
target_link_libraries(MoloVol ${wxWidgets_LIBRARIES})
  # Add custom flag
if(ABS_RESOURCE_PATH)
  target_compile_definitions(MoloVol PUBLIC -DABS_PATH)
endif()

# Keeping this around just so I don't forget the syntax
#  if(OpenMP_CXX_FOUND)
#    target_link_libraries(target PUBLIC OpenMP::OpenMP_CXX)
#  endif()

# Tests
if (MOLOVOL_BUILD_TESTING AND BUILD_TESTING)
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

endif()
