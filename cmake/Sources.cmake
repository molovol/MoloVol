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

if(MOLOVOL_RENDERER AND MOLOVOL_BUILD_GUI)
  list(APPEND LIB_SOURCES "src/render_frame.cpp")
endif()

if(MOLOVOL_BUILD_GUI)
  set(SOURCES ${GUI_SOURCES})
else()
  # For CLI builds, use the library sources directly (includes main function)
  set(SOURCES ${LIB_SOURCES})
endif()

