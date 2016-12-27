#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
out vec3 o_color;

uniform sampler2D s_texture;
uniform sampler2D s_mask;
uniform vec3 u_twinkle;

void main()
{
  // NOTE: Tex co-ords flipped in 't'
	vec3 maskColor = texture2D(s_mask, vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;
	vec3 stars = texture2D(s_texture, vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;

	vec3 mask1 = vec3(maskColor.r) * u_twinkle.r;
	vec3 mask2 = vec3(maskColor.g) * u_twinkle.g;
	vec3 mask3 = vec3(maskColor.b) * u_twinkle.b;

	//o_color = stars;
	o_color = stars * mask1 + stars * mask2 + stars * mask3;
}
