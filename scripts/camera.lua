-- Camera
Camera = class(function(camera, params)
  camera.entity = SuperFX.Entity()  
  SuperFX.init_camera(camera.entity)
  --SuperFX.move_to(camera.entity, 3, 5, 7)
end)

function Camera:render()
  -- no-op
end

function Camera:update()
  arwing = find_object_by_id("arwing")
  vec = SuperFX.get_location(arwing.entity)
  SuperFX.move_to(self.entity, vec.x - 0.2, vec.y - 3, vec.z + 0.5)
  SuperFX.look_at(self.entity, arwing.entity)
end
