# wasm.cmake

# Ensure we're using Emscripten
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "wasm.cmake should only be included when compiling with Emscripten")
endif()

# Set C++ standard for WASM build
set(CMAKE_CXX_EXTENSIONS OFF)

# Emscripten compiler flags
set(WASM_COMPILER_FLAGS
    -s WASM=1
    -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']
    -s ALLOW_MEMORY_GROWTH=1
    -s EXPORTED_FUNCTIONS=['_malloc','_free']
    -fexceptions
)

# Link flags specific to WASM
set(WASM_LINK_FLAGS
    -s ENVIRONMENT=web
    -s MODULARIZE=1
    -s EXPORT_NAME='createMoloVolModule'
    -s NO_EXIT_RUNTIME=1
    -s ASSERTIONS=1
    --bind
    -s EXPORT_ES6=1
    -s USE_ES6_IMPORT_META=0
    
    # Preload resource files
    --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/inputfile/elements.txt@/inputfile/elements.txt
    --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/inputfile/space_groups.txt@/inputfile/space_groups.txt
)

# Apply compiler flags
string(REPLACE ";" " " WASM_COMPILER_FLAGS_STR "${WASM_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WASM_COMPILER_FLAGS_STR}")

# Create WebAssembly target
add_executable(molovol_wasm 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_bindings.cpp
    ${MOLOVOL_SOURCES}  # This should be defined in main CMakeLists.txt
)

# Apply link flags
string(REPLACE ";" " " WASM_LINK_FLAGS_STR "${WASM_LINK_FLAGS}")
set_target_properties(molovol_wasm PROPERTIES
    LINK_FLAGS "${WASM_LINK_FLAGS_STR}"
)

# Copy output files to web directory
add_custom_command(TARGET molovol_wasm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/molovol_wasm.js
        ${CMAKE_CURRENT_SOURCE_DIR}/webserver/molovol_wasm.js
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/molovol_wasm.wasm
        ${CMAKE_CURRENT_SOURCE_DIR}/webserver/molovol_wasm.wasm
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/molovol_wasm.data
        ${CMAKE_CURRENT_SOURCE_DIR}/webserver/molovol_wasm.data
)