require 'scripts/class'
require 'scripts/classes/training1'
require 'scripts/classes/slippy'
require 'scripts/arwing'
require 'scripts/camera'
require 'scripts/ground'
require 'scripts/ring'
require 'scripts/target'
require 'scripts/utils'
require 'scripts/worldspawn'
require 'scripts/intro'

objects = {}
data = {}
paused = false

function find_object_by_id(id)
  for key,object in pairs(objects) do
    if object.id == id then return object end
  end
  print("Couldn't find object" .. id)
  assert(nil)
end

function remove_all_objects()
  for k in pairs(objects) do
    objects[k] = nil
  end
  for k in pairs(data) do
    objects[k] = nil
  end
end

function remove_object(doomed)
  for key,object in pairs(objects) do
    if (object == doomed) then
      table.remove(objects, key)
    end
  end
end

function add_object(object, id)
  assert(id)
  object.id = id
  table.insert(objects, object)
end

function spawn_item(object)
  klass = object.params.class or "Worldspawn"
  spawn = loadstring("return " .. klass .. "(object)")()
  assert(spawn)
  spawn.id = object.id
  spawn.render_order = 0
  table.insert(objects, spawn)
end

function load_level(level)
  objects = {}
  data = dofile("scripts/" .. level).objects
  for k,o in ipairs(data) do
    k = key
    object = o
    if object.params.spawn == "true" then
      spawn_item(object)
    end
  end
  data = remove_if(data, function(object) return object.params.spawn == "true" end)
  sort(data, function(a, b) return a.location[2] < b.location[2] end)
end

function update(dt)
  -- Pause / Unpause
  input = SuperFX.get_input()
  if not paused and input.start_pressed then
    --print("Pausing")
    paused = true 
  elseif paused and input.start_pressed then
    --print("Unpausing")
    paused = false 
  end
  if paused then return end

  -- "Late" spawn new objects
  arwing = find_object_by_id("arwing")
  arwing_position = SuperFX.get_location(arwing.entity).y
  for k,o in ipairs(data) do
    object = o
    if object.location[2] < arwing_position + 30.0 then
      spawn_item(object)
      print("Removing", k, object.id)
      table.remove(data, k)
      break
    end
  end

  for key,object in pairs(objects) do
    object:update(dt, arwing)
  end

  -- Remove objects that have gone behind
  for k,object in ipairs(objects) do
    position = SuperFX.get_location(object.entity).y
    if object.remove_after and position < arwing_position + object.remove_after then
      print("Removing", k, object.id)
      table.remove(objects, k)
      break
    end
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
--load_level("intro_screen.lua")
--load_level("training1.lua")
