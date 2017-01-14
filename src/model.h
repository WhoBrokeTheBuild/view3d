#ifndef VIEW3D_MODEL_H
#define VIEW3D_MODEL_H

#include <shaders.h>
#include <texture.h>
#include <util.h>
#include <view3d.h>

#define MODEL_ATTRID_VERTS 0
#define MODEL_ATTRID_NORMS 1
#define MODEL_ATTRID_TXCDS 2

typedef struct raw_material
{
    char *name;
    vec4f_t ambient;
    vec4f_t diffuse;
    vec4f_t specular;
    float shininess;
    float dissolve;
    char *ambient_map;
    char *diffuse_map;
    char *specular_map;
    char *bump_map;
    char *refl_map;
} raw_material_t;

void raw_material_init(raw_material_t *this);
void raw_material_term(raw_material_t *this);

void raw_material_copy(raw_material_t *dst, raw_material_t *src);

typedef struct raw_mesh
{
    char *name;
    unsigned int count;
    float *verts;
    float *norms;
    float *txcds;
    raw_material_t *mat;
} raw_mesh_t;

void raw_mesh_init(raw_mesh_t *this);
void raw_mesh_term(raw_mesh_t *this);

void raw_mesh_generate_cube(raw_mesh_t *this, float size);
void raw_mesh_generate_sphere(raw_mesh_t *this, float radius, int slices, int stacks);

typedef struct raw_model
{
    unsigned int count;
    raw_mesh_t *meshes;
} raw_model_t;

void raw_model_init(raw_model_t *this);
void raw_model_term(raw_model_t *this);

bool raw_model_load_from_file(raw_model_t *this, const char *filename, const char *name);

#define MATERIAL_

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

bool material_load_from_raw(material_t *this, raw_material_t *raw);

typedef struct mesh
{
    unsigned int count;
    GLuint vao;
    material_t *mat;
} mesh_t;

void mesh_init(mesh_t *this);
void mesh_term(mesh_t *this);

bool mesh_load_from_raw(mesh_t *this, raw_mesh_t *raw);

typedef struct model
{
    unsigned int count;
    mesh_t *meshes;
    GLuint _shader_id;
    shader_data_t *_shader_data;

    GLuint mtl_shininess_loc;
    GLuint mtl_ambient_loc;
    GLuint mtl_diffuse_loc;
    GLuint mtl_specular_loc;
    GLuint tex_ambient_loc;
    GLuint tex_diffuse_loc;
    GLuint tex_specular_loc;

} model_t;

void model_init(model_t *this);
void model_term(model_t *this);

bool model_load_from_file(model_t *this, const char *filename, const char *name, GLuint shader_id, shader_data_t *shader_data);
bool model_load_from_raw(model_t *this, raw_model_t *raw, GLuint shader_id, shader_data_t *shader_data);

void model_draw(model_t *this);

#endif // VIEW3D_MODEL_H
