-- Worldspawn
Worldspawn = class(function(worldspawn, params)
  worldspawn.entity = SuperFX.Entity()
  worldspawn.remove_after = -10.0
  SuperFX.new_mesh_entity(worldspawn.entity, params.url)
  SuperFX.move_to(worldspawn.entity, params.location[1], params.location[2], params.location[3])
  SuperFX.scale(worldspawn.entity, params.scale[1], params.scale[2], params.scale[3])
  SuperFX.rotateXYZ(worldspawn.entity, params.rotation[1], params.rotation[2], params.rotation[3])
end)

function Worldspawn:update()
end

function Worldspawn:arwing_collide(arwing)
  if SuperFX.collide(arwing.entity, self.entity) then
    arwing:take_damage(8)
  end
end

function Worldspawn:projectile_collide(projectile)
  return SuperFX.collide(projectile.entity, self.entity)
end

function Worldspawn:render()
  SuperFX.render(self.entity)
end
