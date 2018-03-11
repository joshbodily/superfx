#pragma once

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#elif __linux__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#endif

#include <gl-matrix.h>
#include <assert.h>
#include <stdbool.h>
