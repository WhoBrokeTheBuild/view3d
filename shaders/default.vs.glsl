#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec3 i_texcoord;

uniform mat4 u_model;
uniform mat4 u_mvp;

out vec4 _position;
out vec4 _normal;
out vec2 _texcoord;

void main()
{
    gl_Position = u_mvp * vec4(i_position, 1);

    _position = u_model * vec4(i_position, 1);
    _normal = u_model * vec4(i_normal, 0);
}
