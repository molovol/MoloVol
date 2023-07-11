
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
