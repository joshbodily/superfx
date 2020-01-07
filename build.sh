#!/bin/bash -ex
set -e

BLENDER_PATH=`which blender`
#swig -lua superfx.i

# Export all models
#~/Downloads/blender-2.78c-OSX_10.6-x86_64/blender.app/Contents/MacOS/blender models/starfox.blend --background --python models/export_ply.py
$BLENDER_PATH models/building1.blend --background --python models/export_ply.py
$BLENDER_PATH models/ring.blend --background --python models/export_ply.py
$BLENDER_PATH models/level.blend --background --python models/export_level.py
lua models/level.lua

#gcc -g `pkg-config --cflags --libs lua glib-2.0 sdl2_ttf` superfx_wrap.c superfx.c parse.c bind.c writepng.c tritri.c text.c console.c utils.c -framework OpenGL -lSDL2 -lgl-matrix -lpng -lm -o superfx

gcc -g `pkg-config --cflags --libs glib-2.0` superfx_wrap.c superfx.c parse.c bind.c writepng.c tritri.c text.c console.c utils.c -lSDL2 -lgl-matrix -lpng -lm -llua5.1 -lSDL2_ttf -lGL -o superfx
#gcc ttfgl.c ttfgl_wrap.c -lGL -lSDL2 -lSDL2_ttf -llua5.1 -o ttfgl

#./superfx
