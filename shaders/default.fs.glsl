#version 330 core

in vec4 _position;
in vec4 _normal;
in vec2 _texcoord;

uniform vec4 u_eye_pos;
uniform vec4 u_light_pos;

uniform float u_mtl_shininess;
uniform vec4 u_mtl_ambient;
uniform vec4 u_mtl_diffuse;
uniform vec4 u_mtl_specular;
uniform sampler2D u_tex_ambient;
uniform sampler2D u_tex_diffuse;
uniform sampler2D u_tex_specular;

layout (location = 0) out vec4 o_color;

void main()
{
    // TODO: Convert to Uniforms
    vec4 light_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vec4 _light_pos = vec4(10.0f, 10.0f, 10.0f, 1.0f);

    vec4 mtl_ambient = (textureSize(u_tex_ambient, 0).x > 0 ? texture(u_tex_ambient, _texcoord) : u_mtl_ambient);
    vec4 mtl_diffuse = (textureSize(u_tex_diffuse, 0).x > 0 ? texture(u_tex_diffuse, _texcoord) : u_mtl_diffuse);
    vec4 mtl_specular = (textureSize(u_tex_specular, 0).x > 0 ? texture(u_tex_specular, _texcoord) : u_mtl_specular);
    float mtl_shininess = u_mtl_shininess;

    vec4 ambient = mtl_ambient;

    vec4 N = normalize(_normal);
    vec4 L = normalize(_light_pos - _position);
    float NdotL = max(dot(N, L), 0);
    vec4 diffuse = NdotL * light_color;

    vec4 V = normalize(u_eye_pos - _position);
    vec4 R = reflect(-L, N);
    float RdotV = max(dot(R, V), 0);
    vec4 specular = pow(RdotV, mtl_shininess) * light_color;

    o_color = (ambient + diffuse + specular) * mtl_diffuse;
}
