#ifndef VIEW3D_MODEL_H
#define VIEW3D_MODEL_H

#include <shaders.h>
#include <view3d.h>

#define MODEL_ATTRID_VERTS 0
#define MODEL_ATTRID_NORMS 1
#define MODEL_ATTRID_TXCDS 2

void calc_normal(vec3f_t normal, vec3f_t a, vec3f_t b, vec3f_t c);

typedef struct
{
    char *name;
    unsigned int count;
    float *verts;
    float *norms;
    float *txcds;
    vec4f_t color;
    char *diff_map;
    // etc...
} raw_mesh_t;

void raw_mesh_init(raw_mesh_t *this);
void raw_mesh_term(raw_mesh_t *this);

void raw_mesh_generate_cube(raw_mesh_t *this, float size);
void raw_mesh_generate_sphere(raw_mesh_t *this, float radius, int slices, int stacks);

typedef struct
{
    unsigned int count;
    raw_mesh_t *meshes;
} raw_model_t;

void raw_model_init(raw_model_t *this);
void raw_model_term(raw_model_t *this);
bool raw_model_load_from_file(raw_model_t *this, const char *filename, const char *name);

typedef struct
{
    unsigned int count;
    GLuint vao;
} mesh_t;

void mesh_init(mesh_t *this);
void mesh_term(mesh_t *this);
bool mesh_load_from_raw(mesh_t *this, raw_mesh_t *raw);

typedef struct
{
    unsigned int count;
    mesh_t *meshes;
    GLuint _shader_id;
    shader_data_t *_shader_data;
} model_t;

void model_init(model_t *this);
void model_term(model_t *this);
bool model_load_from_file(model_t *this, const char *filename, const char *name, GLuint shader_id, shader_data_t *shader_data);
bool model_load_from_raw(model_t *this, raw_model_t *raw, GLuint shader_id, shader_data_t *shader_data);

void model_draw(model_t *this);

#endif // VIEW3D_MODEL_H
