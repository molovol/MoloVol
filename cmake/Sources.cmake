# Set up core sources needed by all builds
set(CORE_SOURCES
  src/atom.cpp
  src/atomtree.cpp
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

# GUI-specific sources
set(GUI_SOURCES
  src/base_guicontrol.cpp
  src/base_constr.cpp
  src/base_event.cpp
  src/base_init.cpp
)

# Set up sources based on build type
if(EMSCRIPTEN)
    set(MOLOVOL_SOURCES 
        ${CORE_SOURCES}
		src/base_cmdline.cpp
    )
elseif(MOLOVOL_BUILD_GUI)
    set(SOURCES 
        ${CORE_SOURCES}
        ${GUI_SOURCES}
    )
else()
    set(SOURCES
        ${CORE_SOURCES}
        src/base_cmdline.cpp
    )
endif()