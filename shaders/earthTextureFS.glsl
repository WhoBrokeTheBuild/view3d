#version 330 core

// We need one out (used to be g_FragColor)
in  vec3 v_vertPos;
in  vec2 v_texCoord;
in  vec3 v_lightDir;
in  vec3 v_norm;
in  vec4 v_shadowCoord;

out vec3 o_color;
uniform sampler2D s_mapTex;
uniform sampler2D s_mapTex_norm;
uniform sampler2D s_specTex;
uniform sampler2DShadow s_shadowMap;
uniform vec2            u_imgDim;
uniform sampler2D s_cloudTex;
uniform sampler2D s_cloudTex2;
uniform vec3 u_eyePos;
uniform vec3 u_lightPos;

void main()
{
  // Normal
  const float epsilon    = 0.00001;
  
  vec3 normRGB = texture2D(s_mapTex_norm, vec2(v_texCoord[0], 1 - v_texCoord[1])).rgb;
  vec3 norm = (normalize(normRGB + v_norm) * 2) - 0.5;

  vec3 lightColor = vec3(0.8f, 0.8f, 0.8f);

  // Emissive
  vec3 Ke = vec3(0.05f, 0.05f, 0.05f);
  vec3 emissive = Ke;

  // Ambient
  vec3 Ka = vec3(0.05f, 0.05f, 0.05f);
  vec3 ambient = Ka * lightColor;

  // Diffuse
  vec3 Kd = texture2D(s_mapTex, vec2(v_texCoord[0], 1 - v_texCoord[1])).rgb;
  vec3 L = normalize(u_lightPos - v_vertPos);
  float diffuseLight = max(dot(norm, L), 0);
  vec3 diffuse = Kd * lightColor * diffuseLight;
 

  // Specular
  float shininess = 5.0f;
  vec3 Ks = texture2D(s_specTex, vec2(v_texCoord[0], 1 - v_texCoord[1])).rgb;
  vec3 V = normalize(u_eyePos - v_vertPos);
  vec3 H = normalize(L + V);
  float specularLight = pow(max(dot(v_norm, H), 0), shininess);
  if (diffuseLight <= 0) specularLight = 0;
  vec3 specular = Ks * lightColor * specularLight;
	 
	
  float diffMult = dot(v_lightDir, v_norm);
  float diffMultClamped = clamp(diffMult, 0.0, 1.0);
	 
	
  vec3 positionLtNDC  = v_shadowCoord.xyz / v_shadowCoord.w;	
  vec2 UVCoords;
  UVCoords.x = positionLtNDC.x;
  UVCoords.y = positionLtNDC.y;
  float z    = positionLtNDC.z + epsilon;
  
  float xOffset = 1.0/u_imgDim.x;
  float yOffset = 1.0/u_imgDim.y;

  float shadowMult = 0.0;
  int   pcfR       = 6;
  
  for (int y = -pcfR; y <= pcfR; y++)
  {
    for (int x = -pcfR; x <= pcfR; x++)
    {
      vec2 offsets = vec2(x * xOffset, y * yOffset);
      vec3 UVC = vec3(UVCoords + offsets, z);
      shadowMult += texture(s_shadowMap, UVC);
    }
  }
  
  float pcfRSq = float(pcfR + pcfR) + 1.0;
  pcfRSq = pcfRSq * pcfRSq;
  shadowMult = (shadowMult / pcfRSq);

  // Cloud
  vec3 clouds = texture2D(s_cloudTex, vec2(v_texCoord[0], 1 - v_texCoord[1])).rgb;
  vec3 transClouds = texture2D(s_cloudTex2, vec2(v_texCoord[0], 1 - v_texCoord[1])).rgb;
	vec3 cloudsFinal = clouds * transClouds;


  // Final Result
  //o_color = emissive + diffuse + specular + (ambient + (shadowMult * diffMultClamped));// + (transClouds * lightColor/2); This sorta adds the clouds not sure if i'm doing it right.
  o_color = diffuse * clouds + (ambient + (shadowMult * diffMultClamped));// + (transClouds * lightColor/2); This sorta adds the clouds not sure if i'm doing it right.

}