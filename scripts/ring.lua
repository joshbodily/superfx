require 'scripts/class'

Ring = class(function(ring, object)
  ring.passed = false

  ring.entity = SuperFX.Entity()
  ring.angle = object.params.angle
  ring.remove_after = -10.0

  SuperFX.new_mesh_entity(ring.entity, object.url)
  SuperFX.move_to(ring.entity, object.location[1], object.location[2], object.location[3])
  SuperFX.scale(ring.entity, object.scale[1], object.scale[2], object.scale[3])
  SuperFX.rotateXYZ(ring.entity, object.rotation[1], object.rotation[2], object.rotation[3])

  ring.collider = SuperFX.Entity()
  SuperFX.new_mesh_entity(ring.collider, "ring_collider.ply")
  SuperFX.move_to(ring.collider, object.location[1], object.location[2], object.location[3])
  SuperFX.scale(ring.collider, object.scale[1], object.scale[2], object.scale[3])
  SuperFX.rotateXYZ(ring.collider, object.rotation[1], object.rotation[2], object.rotation[3])
end)

function Ring:update(dt, arwing)
  --print(arwing)
  SuperFX.rotate(self.entity, 0, 1, 0, self.angle)
end

function Ring:arwing_collide(arwing)
  if not self.passed and SuperFX.collide(arwing.entity, self.collider) then
    print("Passed ring")
    self.passed = true
  end
end

function Ring:render()
  SuperFX.render(self.entity)
  --SuperFX.render(self.collider)
  SuperFX.render_shadow(self.entity)
end
