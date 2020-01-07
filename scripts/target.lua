require 'scripts/class'

Target = class(function(target, object)
  target.entity = SuperFX.Entity()
  target.angle = object.params.angle
  target.remove_after = -10.0
  target.angle = 0.05
  SuperFX.new_mesh_entity(target.entity, object.url)
  SuperFX.move_to(target.entity, object.location[1], object.location[2], object.location[3])
  SuperFX.scale(target.entity, object.scale[1], object.scale[2], object.scale[3])
  SuperFX.rotateXYZ(target.entity, object.rotation[1], object.rotation[2], object.rotation[3])
end)

function Target:update()
  SuperFX.rotate(self.entity, 0, 0, 1, self.angle)
end

function Target:projectile_collide(projectile)
  if SuperFX.collide(projectile.entity, self.entity) then
    self:explode()
  end
end

function Target:render()
  SuperFX.render(self.entity)
end

function Target:explode()
  self.current_position = SuperFX.get_location(self.entity)
  if self.current_position then
    explosion = Explosion({sprite="explosion.png", location={1, 1, 1}, scale={2, 2, 2}, rows=1, columns=12})
    SuperFX.move_to(explosion.entity, self.current_position.x, self.current_position.y, self.current_position.z)
    add_object(explosion, "explosion")
  end
  remove_object(self)
end
