-- Ground
Ground = class(function(ground, params)
  ground.entity = SuperFX.Entity()
  SuperFX.new_mesh_entity(ground.entity, "ground.ply")
  SuperFX.scale(ground.entity, 20, 20, 20)
end)

function Ground:update()
  arwing = find_object_by_id("arwing")
  vec = SuperFX.get_location(arwing.entity)
  y = vec.y % (0.25 * 20) -- Model is 2 units in x and y w/ 8 divisions, so each row is 0.25
  x = vec.x % (0.25 * 20)
  SuperFX.set_translation_world(self.entity, vec.x - x, vec.y + 20 - y, 0)
end

function Ground:render()
  SuperFX.render_points(self.entity)
  SuperFX.move_to(self.entity, vec.x, vec.y, 0)
end
