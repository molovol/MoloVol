cmake_minimum_required(VERSION 3.20)

# Custom options
option(MOLOVOL_ABS_RESOURCE_PATH "Set a platform-dependant, absolute path to the resource directory" OFF)
option(MOLOVOL_OSX_FAT_FILE "Make executable a fat file with architectures x86_64 and arm64" OFF)
option(MOLOVOL_BUILD_TESTING "Enables unit tests so that test source files are compiled along with executable" OFF)

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
set(OSX_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/macOS/icon.icns")
set_source_files_properties(${OSX_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties(${ELEM_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties(${SPACEGROUP_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set(OSX_RESOURCE_FILES ${OSX_ICON_FILE} ${ELEM_FILE} ${SPACEGROUP_FILE})

# Resource files for Debian package
set(DEB_COPYRIGHT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/copyright")
set(DEB_CHANGELOG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/changelog")
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/res/linux/MoloVol.desktop DEB_DESKTOP_FILE)
set(DEB_DESKTOP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/res/linux/MoloVol.desktop)
set(DEB_MAN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/res/linux/molovol.1)

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

# Installation instructions for debian package
if (UNIX AND NOT APPLE)
  include(GNUInstallDirs)

  # Compress changelog
  set(DEB_CHANGELOG_COMPRESSED "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")
  set(DEB_MAN_COMPRESSED "${CMAKE_CURRENT_BINARY_DIR}/molovol.1.gz")

  add_custom_command(
    OUTPUT ${DEB_CHANGELOG_COMPRESSED} ${DEB_MAN_COMPRESSED}
    COMMAND gzip -9 -c -n ${DEB_CHANGELOG_FILE} > ${DEB_CHANGELOG_COMPRESSED}
    COMMAND gzip -9 -c -n ${DEB_MAN_FILE} > ${DEB_MAN_COMPRESSED}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${DEB_CHANGELOG_FILE} ${DEB_MAN_FILE}
    COMMENT "Compressing changelog and manual file"
  )
  add_custom_target(compress ALL DEPENDS ${DEB_CHANGELOG_COMPRESSED} ${DEB_MAN_COMPRESSED})

  # 
  execute_process(COMMAND dpkg --print-architecture COMMAND tr -d '\n' OUTPUT_VARIABLE LINUX_ARCHITECTURE)

  # Install commands so that these files get added to the deb file
  install(TARGETS ${EXE_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(FILES ${ELEM_FILE} ${SPACEGROUP_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${EXE_NAME})
  install(FILES ${DEB_COPYRIGHT_FILE} ${DEB_CHANGELOG_COMPRESSED} DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/${EXE_NAME})
  install(FILES ${DEB_DESKTOP_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
  install(FILES ${DEB_MAN_COMPRESSED} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Packing)
