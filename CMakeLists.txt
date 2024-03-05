cmake_minimum_required(VERSION 3.14)
project(basil C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)

FetchContent_Declare(minifb
    GIT_REPOSITORY https://github.com/emoon/minifb.git
)

set(MINIFB_BUILD_EXAMPLES FALSE CACHE BOOL "")

FetchContent_MakeAvailable(minifb)

set(SOURCES
    src/api.c
    src/basil.c
    src/embed.c
    src/util.c
    src/lib/wren.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} minifb)

if(MSVC)
    target_compile_options(minifb PRIVATE /wd4244)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4996)
else()
    target_link_libraries(${PROJECT_NAME} m)
endif()
