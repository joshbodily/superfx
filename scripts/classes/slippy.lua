require 'scripts/class'

-- Slippy
Slippy = class(function(slippy, params)
  slippy.entity = SuperFX.Entity()
  SuperFX.new_mesh_entity(slippy.entity, "starfox.ply")
  slippy.start = params.location
  SuperFX.move_to(slippy.entity, params.location[1], params.location[2], params.location[3])

  slippy.recording = dofile("scripts/recordings/recording1.lua")
  slippy.startx = slippy.recording[1][5]
  slippy.starty = slippy.recording[1][6]
  slippy.startz = slippy.recording[1][7]
  slippy.recording_seconds = 1

  slippy.debug_timer = 1.0
end)

function Slippy:update(dt)
  seconds = self.recording_seconds
  o1 = self.recording[seconds]
  o2 = self.recording[seconds + 1]
  SuperFX.rotateQuat(self.entity, o1[1], o1[2], o1[3], o1[4])
  quat = SuperFX.slerp(o1[1], o1[2], o1[3], o1[4], o2[1], o2[2], o2[3], o2[4], self.debug_timer);
  SuperFX.rotateQuat(self.entity, quat.x, quat.y, quat.z, quat.w)
  x,y,z = lerp(o1[5], o1[6], o1[7], o2[5], o2[6], o2[7], self.debug_timer)
  pos = SuperFX.get_location(self.entity)
  SuperFX.set_translation_world(self.entity, pos.x + x - self.startx, pos.y + y - self.starty, pos.z + z - self.startz)

  -- Debug
  if self.debug_timer > 1.0 then
    self.recording_seconds = self.recording_seconds + 1
    self.debug_timer = 0.0
  end
  self.debug_timer = self.debug_timer + dt
end

function Slippy:render()
  SuperFX.render(self.entity)
  SuperFX.render_shadow(self.entity)
end
