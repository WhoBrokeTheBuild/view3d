#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 a_vertPos;
in vec2 a_vertTexCoord0;
in mat3 a_vertTBN;

//uniform mat4 u_vp;
//uniform mat4 u_model;
uniform vec3 u_lightPos;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec2 v_texCoord;

void main()
{ 
	vec4 pos = u_proj * vec4(mat3(u_view) * vec3(a_vertPos), 1.0);
	gl_Position = pos.xyzw;
  v_texCoord = a_vertTexCoord0;
}
