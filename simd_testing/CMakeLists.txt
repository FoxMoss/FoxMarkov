add_executable(simd
 main.cpp 
 ../src/node.cpp
)

include(FetchContent)
FetchContent_Declare(
    eve
    GIT_REPOSITORY https://github.com/jfalcou/eve.git
    GIT_TAG main
)
FetchContent_MakeAvailable(eve)

target_link_libraries(simd eve::eve)

add_custom_command(TARGET simd POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")

install(TARGETS simd DESTINATION bin)
