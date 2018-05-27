require 'scripts/class'
require 'scripts/sprite'
require 'scripts/quad'
require 'scripts/projectile'

-- Arwing
Arwing = class(function(arwing, params)
  arwing.entity = SuperFX.Entity()
  SuperFX.new_mesh_entity(arwing.entity, "starfox.ply")
  SuperFX.move_to(arwing.entity, params.location[1], params.location[2], params.location[3])

  arwing.recording = dofile("scripts/recordings/recording1.lua")
  arwing.recording_seconds = 1

  --arwing.logo = Quad({sprite="logo.png"})
  --add_object(arwing.logo, "logo")

  arwing.debug_text = Quad({text="Entities: " .. #objects})
  add_object(arwing.debug_text, "debug_text")
  arwing.debug_timer = 1.0

  arwing.power = Quad({sprite="power.png", x=(256 - 64 - 10)+4, y=8+4, width=64-8, height=16-8})
  add_object(arwing.power, "power")

  border = Quad({sprite="power_border.png", x=(256 - 64 - 10), y=8})
  add_object(border, "power_border")

  arwing.damage_cooldown = 2.0
  arwing.health = 64-8

  -- HUD
  --arwing.hud = Quad({sprite="slippy.png"})
  --add_object(arwing.hud, "hud")

  -- TODO: Create two reticles
  arwing.first_reticle = Sprite({sprite="reticle.png", location={1, 1, 1}, scale={1, 1, 1}})
  --SuperFX.rotate(arwing.first_reticle.entity, 0, 1, 0, 0.3)
  arwing.second_reticle = Sprite({sprite="reticle.png", location={1, 2, 1}, scale={1, 1, 1}})
  add_object(arwing.second_reticle, "reticle2")
  add_object(arwing.first_reticle, "reticle1")

  arwing.yaw = 0.0
  arwing.roll = 0.0
  arwing.pitch = 0.0
  arwing.velocity = 0.1
  arwing.shoot_cooldown = 0.2
end)

function Arwing:update(dt)
  PI_2 = math.pi / 2 -- 90 degrees
  input = SuperFX.get_input()

  self.yaw = self.yaw - input.left_x_axis
  self.pitch = self.pitch + input.left_y_axis
  -- Roll at about 90 degrees per second
  --print(input.left_trigger, input.right_trigger)
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
  if math.abs(input.left_x_axis) < 0.01 then self.yaw = self.yaw * 0.99 end
  if math.abs(input.left_y_axis) < 0.01 then self.pitch = self.pitch * 0.99 end
  if input.left_trigger < -0.9 and input.right_trigger < -0.9 then self.roll = self.roll * 0.90 end
  
  -- Rotations
  pos = SuperFX.get_location(self.entity)
  SuperFX.identity(self.entity)
  SuperFX.rotate(self.entity, 0, 0, 1, self.yaw * PI_2 * 0.02)
  SuperFX.rotate(self.entity, 1, 0, 0, self.pitch * PI_2 * 0.02)
  SuperFX.rotate(self.entity, 0, 1, 0, self.roll)

  -- Translation
  delta = SuperFX.transform(self.entity, 0, self.velocity, 0)
  SuperFX.set_translation_world(self.entity, pos.x + delta.x, pos.y + delta.y, pos.z + delta.z)
  SuperFX.set_transform(self.entity, self.first_reticle.entity)
  SuperFX.set_transform(self.entity, self.second_reticle.entity)

  SuperFX.translate(self.first_reticle.entity, 0, 5, 0)
  SuperFX.translate(self.second_reticle.entity, 0, 10, 0)

  --// Projectiles
  if input.a and self.shoot_cooldown >= 0.2 then
    projectile = Projectile({location={0, 0, 0}})
    SuperFX.set_transform(self.entity, projectile.entity)
    SuperFX.translate(projectile.entity, 0, 1, 0)
    add_object(projectile, "projectile")
    self.shoot_cooldown = 0.0
  end
  self.shoot_cooldown = self.shoot_cooldown + dt

  -- Airbrake
  if input.b then
    self.velocity = 0.1
  else
    self.velocity = 0.2
  end

  if input.back then
    --SuperFX.set_translation_world(self.entity, 0, -20, 0)
    remove_all_objects()
    load_level("level.lua")
  end

  -- Debug
  if self.debug_timer > 1.0 then
    --[[remove_object(self.debug_text)
    self.debug_text = nil
    self.debug_text =  Quad({text="Entities: " .. #objects + 1})
    add_object(self.debug_text, "debug_text")]]--
    self.debug_timer = 0.0
  end
  self.debug_timer = self.debug_timer + dt

  -- Collisions
  for key,object in ipairs(objects) do
    if object.arwing_collide then object:arwing_collide(self) end
  end

  -- Update health bar
  SuperFX.set_scale(self.power.entity, self.health, 8, 0)
  self.damage_cooldown = self.damage_cooldown + dt
end

function Arwing:arwing_collide(arwing)
  -- nop
end

function Arwing:take_damage(quantity)
  if self.damage_cooldown >= 2 then
    self.health = self.health - quantity
    self.damage_cooldown = 0.0
  end
end

function Arwing:render()
  SuperFX.render(self.entity)
  SuperFX.render_shadow(self.entity)
end
