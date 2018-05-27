require 'scripts/class'

Ring = class(function(ring, object)
  ring.entity = SuperFX.Entity()
  ring.angle = object.params.angle
  ring.remove_after = -10.0
  SuperFX.new_mesh_entity(ring.entity, object.url)
  SuperFX.move_to(ring.entity, object.location[1], object.location[2], object.location[3])
  SuperFX.scale(ring.entity, object.scale[1], object.scale[2], object.scale[3])
  SuperFX.rotateXYZ(ring.entity, object.rotation[1], object.rotation[2], object.rotation[3])
end)

function Ring:update()
  SuperFX.rotate(self.entity, 0, 1, 0, self.angle)
end

function Ring:arwing_collide(arwing)
  if SuperFX.collide(arwing.entity, self.entity) then
    print("Ring colliding")
  end
end

function Ring:render()
  SuperFX.render(self.entity)
  SuperFX.render_shadow(self.entity)
end
