require 'scripts/class'

Training1 = class(function(training1, object)
  training1.entity = SuperFX.Entity()
  --SuperFX.new_mesh_entity(arwing.entity, "starfox.ply")

  -- Level Setup
  SuperFX.set_background("sky.png")
end)

function Training1:update()
end

function Training1:render()
  -- no-op
end
