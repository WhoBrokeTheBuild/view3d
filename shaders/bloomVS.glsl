#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 a_vertPos;
in vec2 a_vertTexCoord0;

uniform mat4 u_vp;
uniform mat4 u_model;

out vec2 v_texCoord;

void main()
{ 
  vec4 tmpPos = (u_vp * u_model * vec4(0.0, 0.0, 0.0, 1.0)) + (vec4(
    a_vertPos.x * 1.0 * u_model[0][0], 
    a_vertPos.y * 1.0 * u_model[1][1], 
    a_vertPos.z * 1.0 * u_model[2][2], 
    0.0));
  
  gl_Position = tmpPos;
  v_texCoord = a_vertTexCoord0;
}
