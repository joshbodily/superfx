require 'scripts/class'

Quad = class(function(quad, params)
  quad.render_order = 2
  quad.entity = SuperFX.Entity()
  if params.sprite then
    SuperFX.new_quad_entity(quad.entity, params.sprite)
  elseif params.text then
    SuperFX.new_text_quad_entity(quad.entity, params.text)
  end
  SuperFX.move_to(quad.entity, 10, 10, 0)
  SuperFX.scale(quad.entity, 1, 1, 1)
end)

function Quad:update()
end

function Quad:render()
  SuperFX.render_quad(self.entity, true)
end
