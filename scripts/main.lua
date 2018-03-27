arwing = SuperFX.Entity()
SuperFX.new_mesh_entity(arwing, "starfox.ply")
SuperFX.move_to(arwing, 0, 5, 3)

ground = SuperFX.Entity()
SuperFX.new_mesh_entity(ground, "ground.ply")
SuperFX.scale(ground, 10, 10, 10)

camera = SuperFX.Entity()
SuperFX.init_camera(camera)
SuperFX.move_to(camera, 3, 5, 7)
SuperFX.look_at(camera, arwing)

-- TODO
-- get stick x axis
-- get stick y axis
-- get roll
-- get fire button pressed
-- OR wrap Input type and pass on update
-- set entity transform to identity
-- rotatex, y, & z
-- [x] translate
-- Spawn a projectile if fire pressed

--[[void arwing_update(Entity* this) {
  static float x, y, z = 0.0f;
  static float roll, pitch, velocity = 0.0f;

  yaw -= input.x_axis;
  pitch += input.y_axis;
  roll += input.roll * PI_2 * 0.075;

  // limits
  if (roll > PI_2) roll = PI_2;
  if (roll < -PI_2) roll = -PI_2;

  // return to straight ahead
  if (fabs(input.x_axis) < 0.01f) yaw *= 0.98;
  if (fabs(input.y_axis) < 0.01f) pitch *= 0.97;
  if (fabs(input.roll) < 0.01f) roll *= 0.90;
  
  // Handle input
  mat4_t xform = this->transform;
  mat4_identity(xform);
  mat4_rotateZ(xform, yaw * PI_2 * 0.02, xform);
  mat4_rotateX(xform, pitch * PI_2 * 0.02, xform);
  mat4_rotateY(xform, roll, xform);
  mat4_rotateY(xform, -yaw * PI_2 * 0.001, xform);

  float velocity_vector[3] = {0.0f, 0.08f, 0.0f};
  mat4_multiplyVec3(xform, velocity_vector, NULL);
  x += velocity_vector[0]; y += velocity_vector[1]; z += velocity_vector[2];

  xform[12] = x;
  xform[13] = y;
  xform[14] = z;
  
  // Projectiles
  if (input.fire) {
    spawn_projectile(this->transform);
    input.fire = 0;
  }
}--]]

dofile("models/level.lua")
entities = {}
for k,v in pairs(level.entities) do
  entity = {}
  entity.entity = SuperFX.Entity()
  SuperFX.new_mesh_entity(entity.entity, v.url)
  SuperFX.move_to(entity.entity, v.location[1], v.location[2], v.location[3])
  SuperFX.scale(entity.entity, v.scale[1], v.scale[2], v.scale[3])
  entity.params = v.params

  if v.params.func == "rotate" then
    entity.update = function(self) SuperFX.rotate(self.entity, 0, 1, 0, v.params.rotate_angle) end
  end

  table.insert(entities, entity)
end

function update()
  SuperFX.translate(arwing, 0, 0.075, 0.0)
  SuperFX.move_to(camera, 3, 5, 7)
  SuperFX.look_at(camera, arwing)
  for k,entity in pairs(entities) do
    if entity.update then
      entity:update()
    end
  end
end

function render()
  SuperFX.render(arwing)
  SuperFX.render_shadow(arwing)
  SuperFX.render_points(ground)
  for k,v in pairs(entities) do
    SuperFX.render(v.entity)
    SuperFX.render_shadow(v.entity)
  end
end
