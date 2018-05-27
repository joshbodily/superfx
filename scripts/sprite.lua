require 'scripts/class'

Sprite = class(function(sprite, params)
  sprite.render_order = 1
  sprite.entity = SuperFX.Entity()
  SuperFX.new_sprite_entity(sprite.entity, params.sprite, params.rows or 1, params.columns or 1)
  SuperFX.move_to(sprite.entity, params.location[1], params.location[2], params.location[3])
  SuperFX.scale(sprite.entity, params.scale[1], params.scale[2], params.scale[3])
end)

function Sprite:update(dt)
end

function Sprite:render()
  SuperFX.render_sprite(self.entity)
end
