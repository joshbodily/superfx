# (filepath="", check_existing=True, axis_forward='Y', axis_up='Z', filter_glob="*.ply", use_mesh_modifiers=True, use_normals=True, use_uv_coords=True, use_colors=True, global_scale=1.0)
# bpy.ops.export_mesh.ply("models/__exported.ply")
import bpy
import os

for object in bpy.context.selected_objects:
  # deselect all meshes
  #bpy.ops.object.select_all(action='DESELECT')
  # select the object
  #object.select = True
  # export object with its name as file name
  fPath = str(('models/' + object.name + '.ply'))
  print(fPath)

  #bpy.context.active_object = object
  bpy.ops.export_mesh.ply(filepath=fPath)
