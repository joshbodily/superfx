arwing = SuperFX.Entity()
SuperFX.new_mesh_entity(arwing, "logo.ply")

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


function update(console)
  SuperFX.translate(arwing, 0, 0, 0.016)
  SuperFX.translate(platform, 0, 0, 0.016)
  SuperFX.moveTo(camera, 3, 5, 7)
  SuperFX.lookAt(camera, arwing)

  if console then
    fifo = io.open("/tmp/myfifo")
    local cmd = fifo:read()
    print("Got: " .. cmd)
    loadstring(cmd)()
  end
end

function render()
  SuperFX.render(arwing)
  SuperFX.render(elevator)
  SuperFX.render(platform)
  SuperFX.render_points(ground)
end
