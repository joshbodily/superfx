#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#elif __linux__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#include <gl-matrix.h>

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
