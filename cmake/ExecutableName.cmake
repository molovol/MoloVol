# Executable name is lower case on Linux and normal otherwise
if (UNIX AND NOT APPLE)
  string(TOLOWER ${PROJECT_NAME} LOWER_NAME)
  set(EXE_NAME ${LOWER_NAME})
else()
  set(EXE_NAME ${PROJECT_NAME})
endif()
