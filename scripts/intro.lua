arwing = SuperFX.Entity()
SuperFX.new_mesh_entity(arwing, "starfox.ply")

ground = SuperFX.Entity()
SuperFX.new_mesh_entity(ground, "ground.ply")
SuperFX.scale(ground, 20.0, 20.0, 20.0)

elevator = SuperFX.Entity()
SuperFX.new_mesh_entity(elevator, "elevator.ply")

platform = SuperFX.Entity()
SuperFX.new_mesh_entity(platform, "platform.ply")
SuperFX.translate(platform, 0, 0, -0.5)

camera = SuperFX.Entity()
SuperFX.new_camera(camera)
SuperFX.translate(camera, 3, 5, 7)

function update()
  SuperFX.translate(arwing, 0, 0, 0.016)
  SuperFX.translate(platform, 0, 0, 0.016)
  SuperFX.moveTo(camera, 3, 5, 7)
  SuperFX.lookAt(camera, arwing)
end

function render()
  SuperFX.render(arwing)
  SuperFX.render(elevator)
  SuperFX.render(platform)
  SuperFX.render_points(ground)
end
