
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

if (CMAKE_MACOSX_BUNDLE AND NOT MOLOVOL_ABS_RESOURCE_PATH)
  message(WARNING "The executable inside a macOS application bundle is always compiled with MOLOVOL_ABS_RESOURCE_PATH enabled")
  set(MOLOVOL_ABS_RESOURCE_PATH TRUE)
endif()

option(
  MOLOVOL_RENDERER
  "Enable compilation and linking to wxVTK24" 
  ON
)
