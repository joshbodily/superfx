require 'scripts/class'

Explosion = class(function(sprite, params)
  sprite.render_order = 1
  sprite.quad_index = 0
  sprite.entity = SuperFX.Entity()
  SuperFX.new_sprite_entity(sprite.entity, params.sprite, params.rows or 1, params.columns or 1)
  SuperFX.move_to(sprite.entity, params.location[1], params.location[2], params.location[3])
  SuperFX.scale(sprite.entity, params.scale[1], params.scale[2], params.scale[3])
end)

function Explosion:update(dt)
  self.quad_index = self.quad_index + dt * 8.0
  SuperFX.set_quad_index(self.entity, self.quad_index)
  if self.quad_index >= 12.0 then
    remove_object(self)
  end
end

function Explosion:render()
  SuperFX.render_sprite(self.entity)
end
