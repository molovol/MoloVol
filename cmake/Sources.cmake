# Base sources (non-GUI)
set(BASE_SOURCES
  src/atom.cpp
  src/atomtree.cpp
  src/base_cmdline.cpp
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
)

# GUI-specific sources
set(GUI_SOURCES
  src/base_guicontrol.cpp
  src/base_constr.cpp
  src/base_event.cpp
  src/base_init.cpp
)

set(CLI_SOURCES
    src/base_cmdline.cpp
)

if(MOLOVOL_BUILD_GUI)
    set(SOURCES ${BASE_SOURCES} ${GUI_SOURCES})
else()
    set(SOURCES ${BASE_SOURCES} ${CLI_SOURCES})
endif()

