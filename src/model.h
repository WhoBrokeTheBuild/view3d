#ifndef VIEW3D_MODEL_H
#define VIEW3D_MODEL_H

#include <mdl/mdl.h>
#include <shaders.h>
#include <texture.h>
#include <util.h>
#include <view3d.h>

#define MODEL_ATTRID_VERTS 0
#define MODEL_ATTRID_NORMS 1
#define MODEL_ATTRID_TXCDS 2

typedef struct material
{
  char *name;
  vec4f_t ambient;
  vec4f_t diffuse;
  vec4f_t specular;
  float shininess;
  float dissolve;
  GLuint ambient_map;
  GLuint diffuse_map;
  GLuint specular_map;
  GLuint bump_map;
  GLuint refl_map;
} material_t;

void material_init(material_t *this);
void material_term(material_t *this);

bool material_load_from_mdl(material_t *this, mdl_material_t *mdl);

typedef struct mesh
{
  unsigned int count;
  GLuint vao;
  material_t *mat;
} mesh_t;

void mesh_init(mesh_t *this);
void mesh_term(mesh_t *this);

bool mesh_load_from_mdl(mesh_t *this, mdl_mesh_t *mdl);

typedef struct model
{
  unsigned int count;
  mesh_t *meshes;
  GLuint _shader_id;
  shader_data_t *_shader_data;
  vec3f_t min, max;

  GLuint mtl_shininess_loc;
  GLuint mtl_ambient_loc;
  GLuint mtl_diffuse_loc;
  GLuint mtl_specular_loc;
  GLuint tex_ambient_loc;
  GLuint tex_diffuse_loc;
  GLuint tex_specular_loc;
  GLuint tex_bump_loc;
  GLuint has_tex_ambient_loc;
  GLuint has_tex_diffuse_loc;
  GLuint has_tex_specular_loc;
  GLuint has_tex_bump_loc;
} model_t;

void model_init(model_t *this);
void model_term(model_t *this);

bool model_load_from_file(model_t *this, const char *filename, const char *name, GLuint shader_id, shader_data_t *shader_data);
bool model_load_from_mdl(model_t *this, mdl_model_t *mdl, GLuint shader_id, shader_data_t *shader_data);

void model_draw(model_t *this);

#endif // VIEW3D_MODEL_H
