import bpy
import bmesh
from mathutils import Vector, Matrix

context = bpy.context
scene = context.scene
ob = context.object

copy = ob.copy()
copy.name = "Hull"
copy.data = ob.data.copy()
scene.objects.link(copy)
bpy.context.scene.objects.active = copy

bpy.ops.object.mode_set(mode='EDIT')
bpy.ops.mesh.convex_hull(use_existing_faces=True, join_triangles=False)
mesh = copy.data

bpy.ops.object.mode_set(mode='OBJECT')
bm = bmesh.new()
bm.from_mesh(mesh)
bmesh.ops.triangulate(bm, faces=bm.faces[:], quad_method=0, ngon_method=0)
for v in bm.verts:
      v.co.x += 0.1
bm.to_mesh(mesh)
bm.free()
