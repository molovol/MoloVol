cmake_minimum_required(VERSION 3.20)

# Custom options
option(
  MOLOVOL_ABS_RESOURCE_PATH 
  "Set a platform-dependant, absolute path to the resource directory" 
  OFF
)
option(
  MOLOVOL_OSX_FAT_FILE
  "Make executable a fat file with architectures x86_64 and arm64" 
  OFF
)
option(
  MOLOVOL_BUILD_TESTING
  "Enables unit tests so that test source files are compiled along with executable" 
  OFF
)

# Strip binary for release build
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (CMAKE_MACOSX_BUNDLE AND NOT MOLOVOL_ABS_RESOURCE_PATH)
  message(WARNING "The executable inside a macOS application bundle is always compiled with MOLOVOL_ABS_RESOURCE_PATH enabled")
  set(MOLOVOL_ABS_RESOURCE_PATH TRUE)
endif()

# Minimum macOS version, ignored on other platforms
# Must be set before project()
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS deployment version")

# Set name and version
project(MoloVol VERSION 1.1.0)
# TODO: Consider making executable name lower case on all platforms
if (UNIX AND NOT APPLE)
  string(TOLOWER ${PROJECT_NAME} LOWER_NAME)
  set(EXE_NAME ${LOWER_NAME})
else()
  set(EXE_NAME ${PROJECT_NAME})
endif()

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

# RESOURCE FILES

# Universal resource files
set(ELEM_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inputfile/elements.txt")
set(SPACEGROUP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inputfile/space_groups.txt")

# Resource files for macOS Bundle
set(OSX_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/macOS")
set(OSX_ICON_FILE "${OSX_RES_DIR}/icon.icns")
set(OSX_LICENSE_RTF "${OSX_RES_DIR}/LICENSE.rtf")
set(OSX_DMG_BACKGROUND "${OSX_RES_DIR}/background.png")
set(OSX_DMG_DSSTORE "${OSX_RES_DIR}/DS_Store/.DS_Store")
set_source_files_properties(${OSX_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties(${ELEM_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties(${SPACEGROUP_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set(OSX_RESOURCE_FILES ${OSX_ICON_FILE} ${ELEM_FILE} ${SPACEGROUP_FILE})

# Resource files for Debian package
set(DEB_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/linux")
set(DEB_COPYRIGHT_FILE "${DEB_RES_DIR}/copyright")
set(DEB_CHANGELOG_FILE "${DEB_RES_DIR}/changelog")
file(STRINGS ${DEB_RES_DIR}/MoloVol.desktop DEB_DESKTOP_FILE)
set(DEB_DESKTOP_FILE ${DEB_RES_DIR}/MoloVol.desktop)
set(DEB_MAN_FILE ${DEB_RES_DIR}/molovol.1)
set(DEB_ICON ${DEB_RES_DIR}/molovol.png)

# RESOURCE FILES

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

  # XCode compatibility
set_target_properties(${EXE_NAME} PROPERTIES
  XCODE_GENERATE_SCHEME TRUE
  XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
  # macOS Bundle
set_target_properties(${EXE_NAME} PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
  MACOSX_BUNDLE_EXECUTABLE_NAME ${PROJECT_NAME}
  MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
  MACOSX_BUNDLE_ICON_FILE icon
)

  # Library shenanigans
if(APPLE)
  string(REPLACE "-ltiff" "/usr/local/opt/libtiff/lib/libtiff.a" wxWidgets_LIBRARIES "${wxWidgets_LIBRARIES}")
endif()

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

