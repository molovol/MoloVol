cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Options)
# MOLOVOL_ABS_RESOURCE_PATH
# MOLOVOL_OSX_FAT_FILE
# MOLOVOL_BUILD_TESTING

# Make universal binary, should be called before project()
if(MOLOVOL_OSX_FAT_FILE)
  set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
endif()

# Minimum macOS version, ignored on other platforms
# Must be set before project()
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11 CACHE STRING "Minimum macOS deployment version" FORCE)

# Set name and version
project(MoloVol VERSION 1.2.1)

# TODO: Handle stripping through cpack -- set(CPACK_STRIP_FILES TRUE)
if(NOT APPLE)
  # Strip binary for release build
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

include(Names)
# EXE_NAME
# LIB_NAME

# Specify C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# TODO: This is not within this PR's scope
if(APPLE)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_C_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY /opt/homebrew/opt/libomp/lib/libomp.dylib)
  endif()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY /opt/homebrew/opt/libomp/lib/libomp.dylib)
  endif()
endif()

# wxWidgets
if(MOLOVOL_BUILD_GUI)
  set(wxWidgets_USE_STATIC=ON)
  # TODO: Not ideal to use these absolute paths here
  if (MSVC)
    set(wxWidgets_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/wxWidgets")
    set(wxWidgets_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/wxWidgets/lib/vc_x64_lib-x64-Release-MT")
  endif()
  
  if(MOLOVOL_RENDERER)
    find_package(wxWidgets REQUIRED core base gl OPTIONAL_COMPONENTS net)
  else()
    find_package(wxWidgets REQUIRED core base OPTIONAL_COMPONENTS net)
  endif()
  include(${wxWidgets_USE_FILE})
  
  if(MOLOVOL_RENDERER)
    include(VTKRenderer)
  endif()
endif()

# Add include path, so that header files can be found
include_directories(include)

# Compiler flag for development build
add_compile_options("$<$<NOT:$<CONFIG:RELEASE,MINSIZEREL,RELWITHDEBINFO>>:-DDEBUG>")
if (UNIX)
  add_compile_options(-Wall -Werror -Wno-unused-command-line-argument -Wno-invalid-source-encoding)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if(IPO_SUPPORTED AND CMAKE_BUILD_TYPE_UPPER STREQUAL "RELEASE")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(NOT IPO_SUPPORTED)
  message(WARNING "IPO is not supported: ${IPO_ERROR}")
endif()

include(Sources)
# LIB_SOURCES
# SOURCES

if(MOLOVOL_BUILD_GUI)
  # Create the static library
  add_library(${LIB_NAME} STATIC ${LIB_SOURCES})
  target_include_directories(${LIB_NAME} PUBLIC include)
  
  # Set the same compiler options for the library
  target_compile_options(${LIB_NAME} PRIVATE -Wall -Werror -Wno-unused-command-line-argument -Wno-invalid-source-encoding)
  target_compile_options(${LIB_NAME} PRIVATE "$<$<NOT:$<CONFIG:RELEASE,MINSIZEREL,RELWITHDEBINFO>>:-DDEBUG>")
    
  # Compile definitions
  # All compile definitions are added with the PUBLIC keyword, so that they are
  # automatically passed on to the executable. This way they only need to be added
  # to the library.
  target_compile_definitions(${LIB_NAME} PUBLIC MOLOVOL_GUI)
  
  if(MOLOVOL_RENDERER)
    target_compile_definitions(${LIB_NAME} PUBLIC MOLOVOL_RENDERER) 
  endif()
  
  if(MOLOVOL_ABS_RESOURCE_PATH)
    target_compile_definitions(${LIB_NAME} PUBLIC ABS_PATH)
  endif()
  
else()
  # TODO: Avoid code duplication
  if(MOLOVOL_ABS_RESOURCE_PATH)
    target_compile_definitions(${EXE_NAME} PUBLIC ABS_PATH)
  endif()
endif()

include(Resources)
# ELEM_FILE
# SPACEGROUP_FILE
# OSX_RESOURCE_FILES
# WIN_RESOURCE_FILES

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
if (MSVC)
  add_executable(${EXE_NAME} WIN32 ${SOURCES} ${WIN_RESOURCE_FILES})
else()
  add_executable(${EXE_NAME} ${SOURCES} ${OSX_RESOURCE_FILES})
endif()
# XCode, app bundle and libtiff
include(MacSpecific)

if(MOLOVOL_BUILD_GUI)
  # GUI builds link to the library (since GUI sources are separate)
  target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
  target_link_libraries(${EXE_NAME} PRIVATE ${wxWidgets_LIBRARIES})
  if(MOLOVOL_RENDERER)
    target_link_libraries(${EXE_NAME} PRIVATE ${WXVTK_LIB})
  endif()
else()
  # CLI builds contain all library code directly, no additional linking needed
endif()

# Installation instructions for debian package
if (UNIX AND NOT APPLE)
  include(DebInstall)
elseif(APPLE)
  # This is needed for generation of the dmg file
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/MoloVol.app DESTINATION "." 
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

include(Packing)

# Ensure molovol_test is built if MOLOVOL_BUILD_TESTING is ON
if(MOLOVOL_BUILD_TESTING)
  include(Testing)
endif()
