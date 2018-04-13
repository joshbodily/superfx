require 'scripts/class'
require 'scripts/sprite'
require 'scripts/quad'
require 'scripts/projectile'

-- Arwing
Arwing = class(function(arwing, params)
  arwing.entity = SuperFX.Entity()
  SuperFX.new_mesh_entity(arwing.entity, "starfox.ply")
  SuperFX.move_to(arwing.entity, 0, 5, 3)

  arwing.debug_text = Quad({text="Entities: " .. #objects})
  add_object(arwing.debug_text)
  arwing.debug_timer = 1.0

  -- TODO: Create two reticles
  arwing.first_reticle = Sprite({sprite="reticle.png", location={1, 1, 1}, scale={1, 1, 1}})
  SuperFX.rotate(arwing.first_reticle.entity, 0, 1, 0, 0.3)
  arwing.second_reticle = Sprite({sprite="reticle.png", location={1, 2, 1}, scale={1, 1, 1}})
  add_object(arwing.second_reticle)
  add_object(arwing.first_reticle)

  arwing.yaw = 0.0
  arwing.roll = 0.0
  arwing.pitch = 0.0
  arwing.velocity = 0.0
  arwing.shoot_cooldown = 0.2
end)

function Arwing:update(dt)
  PI_2 = math.pi / 2 -- 90 degrees
  input = SuperFX.get_input()

  self.yaw = self.yaw - input.left_x_axis
  self.pitch = self.pitch + input.left_y_axis
  -- Roll at about 90 degrees per second
  if input.left_trigger > -0.9 then self.roll = self.roll - (input.left_trigger + 0.9) / 1.9 * PI_2 * 0.0166 * 4 end
  if input.right_trigger > -0.9 then self.roll = self.roll + (input.right_trigger + 0.9) / 1.9 * PI_2 * 0.0166 * 4 end
  --[[if input.right_shoulder then
    self.roll = self.roll + PI_2 * 0.075
  elseif input.left_shoulder then
    self.roll = self.roll - PI_2 * 0.075
  end]]

  -- Limits
  if self.roll > PI_2 then self.roll = PI_2 end
  if self.roll < -PI_2 then self.roll = -PI_2 end

  -- Snap back to straight
  if math.abs(input.left_x_axis) < 0.01 then self.yaw = self.yaw * 0.98 end
  if math.abs(input.left_y_axis) < 0.01 then self.pitch = self.pitch * 0.97 end
  if input.left_trigger < -0.9 and input.right_trigger < -0.9 then self.roll = self.roll * 0.90 end
  --if not input.left_shoulder and not input.right_shoulder then self.roll = self.roll * 0.90 end
  
  -- Rotations
  pos = SuperFX.get_location(self.entity)
  SuperFX.identity(self.entity)
  SuperFX.rotate(self.entity, 0, 0, 1, self.yaw * PI_2 * 0.02)
  SuperFX.rotate(self.entity, 1, 0, 0, self.pitch * PI_2 * 0.02)
  SuperFX.rotate(self.entity, 0, 1, 0, self.roll)
  --SuperFX.rotate(self.entity, 0, 1, 0, -self.yaw * PI_2 * 0.001)

  -- Translation
  delta = SuperFX.transform(self.entity, 0, 0.2, 0)
  SuperFX.set_translation_world(self.entity, pos.x + delta.x, pos.y + delta.y, pos.z + delta.z)
  SuperFX.set_translation_world(self.first_reticle.entity, pos.x + delta.x * 10, pos.y + delta.y * 10, pos.z + delta.z * 10)
  SuperFX.set_translation_world(self.second_reticle.entity, pos.x + delta.x * 20, pos.y + delta.y * 20, pos.z + delta.z* 20)

  --// Projectiles
  if input.a and self.shoot_cooldown >= 0.2 then
    projectile = Projectile({location={0, 0, 0}})
    SuperFX.set_transform(self.entity, projectile.entity)
    SuperFX.translate(projectile.entity, 0, 1, 0)
    add_object(projectile, "projectile")
    self.shoot_cooldown = 0.0
  end
  self.shoot_cooldown = self.shoot_cooldown + dt

  if input.back then
    SuperFX.set_translation_world(self.entity, 0, -20, 0)
  end

  -- Debug
  if self.debug_timer > 1.0 then
    remove_object(self.debug_text)
    self.debug_text = nil
    self.debug_text =  Quad({text="Entities: " .. #objects})
    add_object(self.debug_text)
    self.debug_timer = 0.0
  end
  self.debug_timer = self.debug_timer + dt

  -- Collisions
  for key,object in ipairs(objects) do
    if object.arwing_collide then object:arwing_collide(self) end
  end
end

function Arwing:arwing_collide(arwing)
  -- nop
end

function Arwing:render()
  SuperFX.render(self.entity)
  SuperFX.render_shadow(self.entity)
end
