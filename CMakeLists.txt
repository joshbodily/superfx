cmake_minimum_required(VERSION 3.5)
project(SUPERFX)

include_directories(gl-matrix)
link_directories(gl-matrix)

find_package(SDL2 REQUIRED)
find_package(Lua51 REQUIRED)

file(GLOB superfx_SRC "*.c")

add_executable(superfx ${superfx_SRC})
target_compile_options(superfx PRIVATE "-Wno-implicit-function-declaration")

set(superfx_LIBS SDL2 SDL2_ttf lua5.1 m gl-matrix png)
if (APPLE)
  list(APPEND superfx_LIBS "-framework OpenGL")
else()
  list(APPEND superfx_LIBS GL)
endif()

target_link_libraries(superfx ${superfx_LIBS})
