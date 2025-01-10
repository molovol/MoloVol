# Base sources (non-GUI)
set(BASE_SOURCES
  src/atom.cpp
  src/atomtree.cpp
  src/base_cmdline.cpp
  src/base_constr.cpp
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
)

# GUI-specific sources
set(GUI_SOURCES
  src/base_guicontrol.cpp
  src/base_event.cpp
  src/base_init.cpp
  src/controller.cpp
)

# Combine sources based on GUI option
if(MOLOVOL_BUILD_GUI)
  set(SOURCES ${BASE_SOURCES} ${GUI_SOURCES})
else()
  set(SOURCES ${BASE_SOURCES})
endif()