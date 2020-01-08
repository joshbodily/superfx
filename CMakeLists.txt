cmake_minimum_required(VERSION 3.5)
project(SUPERFX)

include_directories(gl-matrix)
link_directories(gl-matrix)

set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
find_package(PkgConfig)
pkg_check_modules(SDL2_ttf REQUIRED SDL2_ttf)
find_package(SDL2)
find_package(Lua51 REQUIRED)
find_package(PNG REQUIRED)

file(GLOB superfx_SRC "*.c")

add_executable(superfx ${superfx_SRC})
include_directories(superfx ${SDL2_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS})
target_compile_options(superfx PRIVATE "-Wno-implicit-function-declaration" "-Wno-deprecated-declarations")

set(CMAKE_EXE_LINKER_FLAGS "-L${SDL2_ttf_LIBDIR}")
set(superfx_LIBS ${SDL2_LIBRARIES} SDL2_ttf lua5.1 m gl-matrix png)
if (APPLE)
  list(APPEND superfx_LIBS "-framework OpenGL")
else()
  list(APPEND superfx_LIBS GL)
endif()

target_link_libraries(superfx ${superfx_LIBS})
