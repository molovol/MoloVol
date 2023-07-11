
# TODO: Consider making executable name lower case on all platforms
if (UNIX AND NOT APPLE)
  string(TOLOWER ${PROJECT_NAME} LOWER_NAME)
  set(EXE_NAME ${LOWER_NAME})
else()
  set(EXE_NAME ${PROJECT_NAME})
endif()
