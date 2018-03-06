#!/bin/bash -ex
set -e

swig -lua superfx.i

# Export all models
#~/Downloads/blender-2.78c-OSX_10.6-x86_64/blender.app/Contents/MacOS/blender models/starfox.blend --background --python models/export_ply.py
~/Downloads/blender-2.78c-OSX_10.6-x86_64/blender.app/Contents/MacOS/blender models/building1.blend --background --python models/export_ply.py
~/Downloads/blender-2.78c-OSX_10.6-x86_64/blender.app/Contents/MacOS/blender models/ring.blend --background --python models/export_ply.py
~/Downloads/blender-2.78c-OSX_10.6-x86_64/blender.app/Contents/MacOS/blender models/level.blend --background --python models/export_level.py
lua models/level.lua

export PKG_CONFIG_PATH=/usr/local/Cellar/lua/5.2.4_1/lib/pkgconfig:/usr/local/Cellar/glib/2.44.0/lib/pkgconfig:/usr/local/Cellar/sdl2_ttf/2.0.14/lib/pkgconfig/
ruby models/parse_scene.rb models/level.dae > scripts/level.lua
gcc -g `pkg-config --cflags --libs lua glib-2.0 sdl2_ttf` superfx_wrap.c superfx.c parse.c bind.c writepng.c tritri.c text.c console.c utils.c -framework OpenGL -lSDL2 -lgl-matrix -lpng -lm -o superfx
./superfx
