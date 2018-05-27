
-- Intro
Intro = class(function(intro, params)
  sprite = Sprite({sprite="logo.png", location={0, 0, 0}, scale={3, 1, 3}})
  add_object(sprite, "logo")

  intro.entity = SuperFX.Entity()
  SuperFX.new_mesh_entity(intro.entity, params.url)

  intro.logo = Quad({sprite="test.png", x=10, y=10})
  add_object(intro.logo, "test64x64")

  intro.power = Quad({sprite="power_border.png", x=(256 - 64 - 10), y=10})
  add_object(intro.power, "power_border")

  intro.press_start = Quad({text="Press Start", x=84, y=10})
  add_object(intro.press_start, "press_start")
end)

function Intro:update()
  SuperFX.rotate(self.entity, 0, 0, 1, 0.02)

  input = SuperFX.get_input()
  if input.start then
    remove_all_objects()
    load_level("level.lua")
  end
end

function Intro:render()
  SuperFX.render(self.entity)
end
