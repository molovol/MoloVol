# RESOURCE FILES
# Universal resource files
set(ELEM_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inputfile/elements.txt")
set(SPACEGROUP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inputfile/space_groups.txt")
set(LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(README_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Third party licenses and copyright notices
if(MOLOVOL_RENDERER)
  set(TPL_VTK "${CMAKE_CURRENT_SOURCE_DIR}/external/VTK/Copyright.txt")
  set(TPL_VTK_COPY "${CMAKE_BINARY_DIR}/VTK.txt")
  configure_file(${TPL_VTK} ${TPL_VTK_COPY} COPYONLY)
endif()

# Resource files for macOS Bundle
set(OSX_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/macOS")
set(OSX_ICON_FILE "${OSX_RES_DIR}/icon.icns")
set(OSX_LICENSE_RTF "${OSX_RES_DIR}/LICENSE.rtf")
set(OSX_DMG_BACKGROUND "${OSX_RES_DIR}/background.png")
set(OSX_DMG_DSSTORE "${OSX_RES_DIR}/DS_Store/.DS_Store")
# TODO: Why is icon file moved to app bundle like this?
set_source_files_properties(${OSX_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties(${ELEM_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties(${SPACEGROUP_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

if(MOLOVOL_RENDERER)
  set_source_files_properties(${TPL_VTK_COPY} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/licenses")
  set(OSX_RESOURCE_FILES ${OSX_ICON_FILE} ${ELEM_FILE} ${SPACEGROUP_FILE} ${TPL_VTK_COPY})
else()
  set(OSX_RESOURCE_FILES ${OSX_ICON_FILE} ${ELEM_FILE} ${SPACEGROUP_FILE})
endif()

# Resource files for Debian package
set(DEB_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/linux")
set(DEB_COPYRIGHT_FILE "${DEB_RES_DIR}/copyright")
set(DEB_CHANGELOG_FILE "${DEB_RES_DIR}/changelog")
file(STRINGS ${DEB_RES_DIR}/MoloVol.desktop DEB_DESKTOP_FILE)
set(DEB_DESKTOP_FILE ${DEB_RES_DIR}/MoloVol.desktop)
set(DEB_MAN_FILE ${DEB_RES_DIR}/molovol.1)
set(DEB_ICON ${DEB_RES_DIR}/molovol.png) 

# Resource files for Windows
set(WIN_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/windows")
set(WIN_RESOURCE_FILES "${WIN_RES_DIR}/resource.rc")
set(WIN_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/windows/icon.ico")
set(WIN_LICENSE_RTF "${WIN_RES_DIR}/LICENSE.rtf")

# Example files
set(INPUTDIR inputfile)
set(EXAMPLE_FILES ${INPUTDIR}/example_C60.cif ${INPUTDIR}/example_C60.xyz ${INPUTDIR}/example_C60.pdb)
