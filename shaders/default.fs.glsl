#version 330 core

in vec4 _position;
in vec4 _normal;
in vec2 _texcoord;

uniform vec4 u_eye_pos;
uniform vec4 u_light_pos;

uniform vec4 u_mtl_diffuse;

uniform sampler2D u_tex_diffuse;

layout (location = 0) out vec4 o_color;

void main()
{
    // TODO: Convert to Uniforms
    vec4 light_color = vec4(1.0f, 0.6f, 0.6f, 1.0f);
    vec4 mtl_emissive = vec4(0.0f);

    vec4 mtl_specular = vec4(0.3500, 0.3500, 0.3500, 1.0f);
    float mtl_shininess = 35;
    vec4 ambient = vec4(0.1f, 0.1f, 0.1f, 1.0f);
    vec4 _light_pos = vec4(10.0f, 10.0f, 10.0f, 1.0f);

    vec4 emissive = mtl_emissive;

    vec4 N = normalize(_normal);
    vec4 L = normalize(_light_pos - _position);
    float NdotL = max(dot(N, L), 0);
    vec4 diffuse = NdotL * light_color;

    vec4 V = normalize(u_eye_pos - _position);
    vec4 R = reflect(-L, N);
    float RdotV = max(dot(R, V), 0);
    vec4 specular = pow(RdotV, mtl_shininess) * light_color * mtl_specular;

    o_color = (emissive + ambient + diffuse + specular) * (u_mtl_diffuse + texture(u_tex_diffuse, _texcoord));
    //o_color = vec4(_texcoord.x, _texcoord.y, 0.0f, 1.0f);
}
