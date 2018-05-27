-- Utils

function remove_if(t, func)
  ret = {}
  for key,object in pairs(t) do
    if not func(object) then
      table.insert(ret, object)
    end
  end
  return ret
end

function print_all(t)
  for key,object in ipairs(t) do
    print(key, " : ", object)
  end
end

function sort(t, func)
  table.sort(t, func)
end

function lerp(x1, y1, z1, x2, y2, z2, t)
  return (1.0 - t) * x1 + t * x2,
         (1.0 - t) * y1 + t * y2,
         (1.0 - t) * z1 + t * z2
end
