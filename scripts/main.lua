require 'scripts/class'
require 'scripts/arwing'
require 'scripts/camera'
require 'scripts/ground'
require 'scripts/worldspawn'
require 'scripts/ring'

objects = {}

function find_object_by_id(id)
  for key,object in pairs(objects) do
    if object.id == id then return object end
  end
  assert(nil)
end

function remove_object(doomed)
  for key,object in pairs(objects) do
    if (object == doomed) then
      table.remove(objects, key)
    end
  end
end

function add_object(object, id)
  object.id = id
  table.insert(objects, object)
end

function load_level(level)
  objects = {}
  data = dofile("scripts/" .. level)
  for k,o in pairs(data.objects) do
    key = k
    object = o

    klass = object.params.class or "Worldspawn"
    spawn = loadstring("return " .. klass .. "(object)")()
    assert(spawn)
    spawn.id = object.id
    spawn.render_order = 0
    table.insert(objects, spawn)
  end
end

function update(dt)
  for key,object in pairs(objects) do
    object:update(dt)
  end
  -- Do collision detection
  for key,object in pairs(objects) do
  end
end

function spairs(t, order)
  -- collect the keys
  local keys = {}
  for k in pairs(t) do keys[#keys+1] = k end

  -- if order function given, sort by it by passing the table and keys a, b,
  -- otherwise just sort the keys 
  if order then
    table.sort(keys, function(a,b) return order(t, a, b) end)
  else
    table.sort(keys)
  end

  -- return the iterator function
  local i = 0
  return function()
    i = i + 1
    if keys[i] then
      return keys[i], t[keys[i]]
    end
  end
end

function render()
  for key,object in spairs(objects, 
    function(t,a,b)
      if t[a].render_order == t[b].render_order then
        pos1 = SuperFX.get_location(t[a].entity)
        pos2 = SuperFX.get_location(t[b].entity)
        return pos2.y < pos1.y
      else
        return t[a].render_order < t[b].render_order 
      end
    end) do
    object:render()
  end
end

load_level("level.lua")
