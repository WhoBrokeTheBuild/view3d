#include "model.h"
#include <model_blend.h>
#include <model_fbx.h>
#include <model_obj.h>

typedef struct
{
    const char *ext;
    bool (*func)(raw_model_t *, const char *, const char *);

} model_loader_t;

model_loader_t g_model_loaders[] = {
    { ".obj", &raw_model_load_from_obj },
    { ".fbx", &raw_model_load_from_fbx },
    { ".blend", &raw_model_load_from_blend },
    { NULL, NULL }
};

void calc_normal(vec3f_t normal, vec3f_t a, vec3f_t b, vec3f_t c)
{
    int i;
    vec3f_t tmpa, tmpb;

    for (i = 0; i < 3; ++i)
    {
        tmpa[i] = b[i] - a[i];
        tmpb[i] = c[i] - a[i];
    }

    normal[0] = tmpa[1] * tmpb[2] - tmpa[2] * tmpb[1];
    normal[1] = tmpa[2] * tmpb[0] - tmpa[0] * tmpb[2];
    normal[1] = tmpa[0] * tmpb[1] - tmpa[1] * tmpb[0];
}

void raw_mesh_init(raw_mesh_t *this)
{
    this->name = NULL;
    this->count = 0;
    this->verts = NULL;
    this->norms = NULL;
    this->txcds = NULL;
    vec4f_init(this->color, 0.0f);
    this->diff_map = NULL;
}

void raw_mesh_term(raw_mesh_t *this)
{
    free(this->name);
    this->name = NULL;
    this->count = 0;
    free(this->verts);
    this->verts = NULL;
    free(this->norms);
    this->norms = NULL;
    free(this->txcds);
    this->txcds = NULL;
    free(this->diff_map);
    this->diff_map = NULL;

    raw_mesh_init(this);
}

void raw_mesh_generate_cube(raw_mesh_t *this, float size)
{
    int i;
    float cube_verts[] = {
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };

    CHECK(this, "this is NULL");

    for (i = 0; i < sizeof(cube_verts); ++i)
    {
        cube_verts[i] *= size;
    }

    this->count = 36;
    this->verts = malloc(this->count * sizeof(vec3f_t));
    memcpy(this->verts, cube_verts, this->count * sizeof(vec3f_t));

// TODO: Normals & Tex Coords

error:;
}

void raw_mesh_generate_sphere(raw_mesh_t *this, float radius, int slices, int stacks)
{
    int i;
    int index;
    vec3f_t tmp3;
    vec3f_t tmp2;

    CHECK(this, "this is NULL");

    this->count = stacks * slices;
    this->verts = malloc(this->count * sizeof(vec3f_t));
    this->norms = malloc(this->count * sizeof(vec3f_t));
    this->txcds = malloc(this->count * sizeof(vec2f_t));

    for (i = 0; i <= stacks; ++i)
    {
        // V texture coordinate.
        tmp2[1] = i / (float)stacks;
        float phi = tmp2[1] * GLMM_PI;

        for (int j = 0; j <= slices; ++j)
        {
            // U texture coordinate.
            tmp2[0] = j / (float)slices;
            float theta = tmp2[0] * GLMM_2PI;

            tmp3[0] = cosf(theta) * sinf(phi);
            tmp3[1] = cosf(phi);
            tmp3[2] = sinf(theta) * sinf(phi);

            index = (i * slices) + j;

            vec3f_mul_scalar(this->verts + index, tmp3, radius);
            vec3f_copy(this->norms + index, tmp3);
            vec2f_copy(this->txcds + index, tmp2);
        }
    }

error:;
}

void raw_model_init(raw_model_t *this)
{
    CHECK(this, "this is NULL");

    this->count = 0;
    this->meshes = NULL;

error:;
}

void raw_model_term(raw_model_t *this)
{
    int i;
    for (i = 0; i < this->count; ++i)
    {
        raw_mesh_term(&this->meshes[i]);
    }
    free(this->meshes);
    this->meshes = NULL;
    this->count = 0;
}

bool raw_model_load_from_file(raw_model_t *this, const char *filename, const char *name)
{
    int i = 0;
    char *pch = NULL;

    pch = strrchr(filename, '.');
    CHECK(pch, "No file extension found for '%s'", filename);

    for (i = 0; g_model_loaders[i].ext; ++i)
    {
        if (strcmp(pch, g_model_loaders[i].ext) == 0)
        {
            return (*g_model_loaders[i].func)(this, filename, name);
        }
    }

    LOG_ERR("Unable to find model loader for '%s'", filename);

error:

    return false;
}

void mesh_init(mesh_t *this)
{
    this->count = 0;
    this->vao = 0;
}

void mesh_term(mesh_t *this)
{
    this->count = 0;
    glDeleteVertexArrays(1, &this->vao);
    this->vao = 0;
}

bool mesh_load_from_raw(mesh_t *this, raw_mesh_t *raw)
{
    this->count = raw->count;

    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    GLuint vbos[4];
    glGenBuffers(4, vbos);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * raw->count * 3, raw->verts, GL_STATIC_DRAW);
    glVertexAttribPointer(MODEL_ATTRID_VERTS, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(MODEL_ATTRID_VERTS);

    if (raw->norms)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * raw->count * 3, raw->norms, GL_STATIC_DRAW);
        glVertexAttribPointer(MODEL_ATTRID_NORMS, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(MODEL_ATTRID_NORMS);
    }

    if (raw->txcds)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * raw->count * 2, raw->txcds, GL_STATIC_DRAW);
        glVertexAttribPointer(MODEL_ATTRID_TXCDS, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(MODEL_ATTRID_TXCDS);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

void model_init(model_t *this)
{
    this->count = 0;
    this->meshes = NULL;
    this->_shader_id = 0;
    this->_shader_data = NULL;
}

void model_term(model_t *this)
{
    int i;
    for (i = 0; i < this->count; ++i)
    {
        mesh_term(&this->meshes[i]);
    }
    this->count = 0;
    free(this->meshes);
    this->meshes = NULL;
    this->_shader_id = 0;
}

bool model_load_from_file(model_t *this, const char *filename, const char *name, GLuint shader_id, shader_data_t *shader_data)
{
    bool ret;
    raw_model_t raw;

    raw_model_init(&raw);
    raw_model_load_from_file(&raw, filename, name);
    ret = model_load_from_raw(this, &raw, shader_id, shader_data);

    raw_model_term(&raw);

    return ret;
}

bool model_load_from_raw(model_t *this, raw_model_t *raw, GLuint shader_id, shader_data_t *shader_data)
{
    int i;

    this->_shader_id = shader_id;
    if (shader_data)
    {
        this->_shader_data = malloc(sizeof(shader_data_t));
        memcpy(this->_shader_data, shader_data, sizeof(shader_data_t));
        shader_data_init(this->_shader_data, this->_shader_id);
    }

    this->count = raw->count;
    this->meshes = malloc(sizeof(mesh_t) * raw->count);
    for (i = 0; i < raw->count; ++i)
    {
        mesh_init(&this->meshes[i]);
        CHECK(mesh_load_from_raw(&this->meshes[i], &raw->meshes[i]), "Failed to load mesh");
    }

    return true;

error:

    return false;
}

void model_draw(model_t *this)
{
    int i;

    glUseProgram(this->_shader_id);

    if (this->_shader_data)
    {
        shader_data_bind(this->_shader_data);
    }

    for (i = 0; i < this->count; ++i)
    {
        glBindVertexArray(this->meshes[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, this->meshes[i].count);
    }
}
