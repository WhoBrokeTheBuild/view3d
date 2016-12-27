#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
in  vec3 v_norm;
in  vec3 v_lightDir;
in  vec3 v_vertPos;
in  vec3 normalPass;
out vec3 o_color;
uniform sampler2D s_texture;
uniform vec3 u_eyePos;

void main()
{
  vec3 color = vec3(1.0, 1.0, 1.0);
	vec3 silColor = vec3(0.0, 0.0, 0.0);

	vec3 eyePosVert = u_eyePos - v_vertPos;

	float sil = max(dot(normalize(v_norm), eyePosVert), 0.0);
	if (sil < 1.2)
	{ 
		o_color = silColor;
	}
	else
	{
		float intensity = dot(v_lightDir, normalize(v_norm));
		intensity = clamp(intensity, 0.1f, 1.0f);

		if (intensity > 0.95)
			color = vec3(0.5, 0.5, 1.0) * color;
		else if (intensity > 0.5)
			color = vec3(0.3, 0.3, 0.6) * color;
		else if (intensity > 0.25)
			color = vec3(0.2, 0.2, 0.4) * color;
		else
			color = vec3(0.1, 0.1, 0.2) * color;
		
		o_color = color;
	}
}
