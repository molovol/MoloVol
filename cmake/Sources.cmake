# Base sources (includes CLI parsing for both GUI and non-GUI)
set(LIB_SOURCES
  src/atom.cpp
  src/atomtree.cpp
  src/cavity.cpp
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
  src/controller.cpp
  src/base_cmdline.cpp
)

# GUI-specific sources
set(GUI_SOURCES
  src/base_guicontrol.cpp
  src/base_constr.cpp
  src/base_event.cpp
  src/base_init.cpp
)

# CLI builds use LIB_SOURCES directly (no separate CLI_SOURCES needed)
# Create the static library
add_library(molovol_lib STATIC ${LIB_SOURCES})
target_include_directories(molovol_lib PUBLIC include)

# Set the same compiler options for the library
target_compile_options(molovol_lib PRIVATE -Wall -Werror -Wno-unused-command-line-argument -Wno-invalid-source-encoding)
target_compile_options(molovol_lib PRIVATE "$<$<NOT:$<CONFIG:RELEASE,MINSIZEREL,RELWITHDEBINFO>>:-DDEBUG>")

# Define MOLOVOL_GUI for the library when building with GUI
if(MOLOVOL_BUILD_GUI)
    target_compile_definitions(molovol_lib PRIVATE MOLOVOL_GUI)
endif()

if(MOLOVOL_BUILD_GUI)
    set(SOURCES ${GUI_SOURCES})
else()
    # For CLI builds, use the library sources directly (includes main function)
    set(SOURCES ${LIB_SOURCES})
endif()
