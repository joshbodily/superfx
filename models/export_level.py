import bpy
import re

scene = bpy.data.scenes[0].name
fPath = str(('scripts/' + scene + '.lua'))
print(fPath)

file = open(fPath, 'w')

file.write("return {\n");
file.write("  objects={\n");

def emit(obj):
  params = {}
  for key,val in obj.items():
    if key != "_RNA_UI":  
      params[key] = val
  param_str = re.sub(r"'([^']*)': ", r"\1=", str(params))

  scale = obj.scale
  rotation = obj.rotation_euler
  location = obj.location

  file.write("    {id=\"%s\", url=\"%s.ply\", location={%f, %f, %f}, scale={%f, %f, %f}, rotation={%f, %f, %f}, params=%s},\n" % (obj.name, obj.data.name, location[0], location[1], location[2], scale[0], scale[1], scale[2], rotation[0], rotation[1], rotation[2], param_str)) # mesh name


for obj in bpy.data.objects:
  if obj.library is None:
    emit(obj)
    print(obj)

file.write("  }\n");
file.write("}\n");
file.close()
