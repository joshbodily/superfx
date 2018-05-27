require 'scripts/class'

Quad = class(function(quad, params)
  quad.render_order = 2
  quad.entity = SuperFX.Entity()
  if params.sprite then
    size = SuperFX.new_quad_entity(quad.entity, params.sprite)
    quad.invert = false
  elseif params.text then
    size = SuperFX.new_text_quad_entity(quad.entity, params.text)
    quad.invert = true
  end

  width = params.width or size.width
  height = params.height or size.height
  x = params.x or 0
  y = params.y or 0
  print(x, y, width, height)

  if params.width and params.height then
    SuperFX.move_to(quad.entity, x, y, 0)
    SuperFX.scale(quad.entity, width, height, 1)
  else
    SuperFX.move_to(quad.entity, x, y, 0)
    SuperFX.scale(quad.entity, width, height, 1)
  end
end)

function Quad:update()
end

function Quad:render()
  SuperFX.render_quad(self.entity, self.invert)
end
