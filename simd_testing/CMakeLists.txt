add_executable(simd
 main.cpp 
 ../src/node.cpp
)

include(FetchContent)

include_directories(../src/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx2 ")

add_custom_command(TARGET simd POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")

install(TARGETS simd DESTINATION bin)
