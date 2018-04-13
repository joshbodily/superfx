require 'scripts/explosion'

-- Projectile
Projectile = class(function(projectile, params)
  projectile.render_order = 0
  projectile.entity = SuperFX.Entity()
  SuperFX.new_mesh_entity(projectile.entity, "beam.ply")
  --SuperFX.move_to(projectile.entity, params.location[1], params.location[2], params.location[3])
  projectile.life_time = 0.0
end)

function Projectile:update(dt)
  self.life_time = self.life_time + dt
  if self.life_time > 3.0 then
    remove_object(self)
    return
  end

  SuperFX.translate(self.entity, 0, 0.6, 0)
  vec = SuperFX.get_location(self.entity)
  if vec.z < 0 then
    self:explode()
  end

  for key,object in ipairs(objects) do
    if object.projectile_collide then 
      if object:projectile_collide(self) then
        self:explode()
      end
    end
  end

  self.current_position = SuperFX.get_location(self.entity)
end

function Projectile:render()
  SuperFX.render(self.entity)
  SuperFX.render_shadow(self.entity)
end

function Projectile:explode()
  explosion = Explosion({sprite="explosion.png", location={1, 1, 1}, scale={1, 1, 1}})
  --SuperFX.set_transform(self.entity, explosion.entity)
  SuperFX.move_to(explosion.entity, self.current_position.x, self.current_position.y, self.current_position.z)
  add_object(explosion, "explosion")
  remove_object(self)
end

