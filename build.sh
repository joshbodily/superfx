#!/bin/bash -ex
set -e

swig -lua superfx.i
# Update the include path
sed -i '' 's/lua.h/lua5.1\/lua.h/' superfx_wrap.c
sed -i '' 's/lauxlib.h/lua5.1\/lauxlib.h/' superfx_wrap.c

# Export all models
#BLENDER=`which blender`
BLENDER=/Applications/blender.app/Contents/MacOS/blender
#$BLENDER models/building1.blend --background --python models/export_ply.py
#$BLENDER models/ring.blend --background --python models/export_ply.py

# Export levels
$BLENDER models/level.blend --background --python models/export_level.py
$BLENDER models/training1.blend --background --python models/export_level.py

#lua models/level.lua
