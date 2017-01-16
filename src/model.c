#include "model.h"
#include <model_blend.h>
#include <model_fbx.h>
#include <model_obj.h>
#include <string.h>

typedef struct model_loader
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

void raw_material_init(raw_material_t *this)
{
    CHECK(this, "this is NULL");

    this->name = NULL;
    vec4f_init(this->ambient, 0.0f);
    this->ambient[3] = 1.0f;
    vec4f_init(this->diffuse, 0.0f);
    this->diffuse[3] = 1.0f;
    vec4f_init(this->specular, 0.0f);
    this->specular[3] = 1.0f;
    this->shininess = 0.0f;
    this->dissolve = 0.0f;
    this->ambient_map = NULL;
    this->diffuse_map = NULL;
    this->specular_map = NULL;
    this->bump_map = NULL;
    this->refl_map = NULL;

error:;
}

void raw_material_term(raw_material_t *this)
{
    CHECK(this, "this is NULL");

    free(this->name);
    this->name = NULL;
    vec4f_init(this->ambient, 0.0f);
    this->ambient[3] = 1.0f;
    vec4f_init(this->diffuse, 0.0f);
    this->diffuse[3] = 1.0f;
    vec4f_init(this->specular, 0.0f);
    this->specular[3] = 1.0f;
    this->shininess = 0.0f;
    this->dissolve = 0.0f;
    free(this->ambient_map);
    free(this->diffuse_map);
    free(this->specular_map);
    free(this->bump_map);
    free(this->refl_map);
    this->ambient_map = NULL;
    this->diffuse_map = NULL;
    this->specular_map = NULL;
    this->bump_map = NULL;
    this->refl_map = NULL;

error:;
}

void raw_material_copy(raw_material_t *dst, raw_material_t *src)
{
    CHECK(dst, "dst is NULL");
    CHECK(dst, "src is NULL");

    raw_material_term(dst);

    if (src->name)
    {
        dst->name = _strndup(src->name, V3D_MAX_NAME_LEN);
        CHECK_MEM(dst->name);
    }
    vec4f_copy(dst->ambient, src->ambient);
    vec4f_copy(dst->diffuse, src->diffuse);
    vec4f_copy(dst->specular, src->specular);
    dst->dissolve = src->dissolve;
    dst->shininess = src->shininess;
    if (src->ambient_map)
    {
        dst->ambient_map = _strndup(src->ambient_map, V3D_MAX_PATH_LEN);
        CHECK_MEM(dst->ambient_map);
    }
    if (src->diffuse_map)
    {
        dst->diffuse_map = _strndup(src->diffuse_map, V3D_MAX_PATH_LEN);
        CHECK_MEM(dst->diffuse_map);
    }
    if (src->specular_map)
    {
        dst->specular_map = _strndup(src->specular_map, V3D_MAX_PATH_LEN);
        CHECK_MEM(dst->specular_map);
    }
    if (src->bump_map)
    {
        dst->bump_map = _strndup(src->bump_map, V3D_MAX_PATH_LEN);
        CHECK_MEM(dst->bump_map);
    }
    if (src->refl_map)
    {
        dst->refl_map = _strndup(src->refl_map, V3D_MAX_PATH_LEN);
        CHECK_MEM(dst->refl_map);
    }

error:;
}

void raw_mesh_init(raw_mesh_t *this)
{
    CHECK(this, "this is NULL");

    this->name = NULL;
    this->count = 0;
    this->verts = NULL;
    this->norms = NULL;
    this->txcds = NULL;
    this->mat = NULL;

error:;
}

void raw_mesh_term(raw_mesh_t *this)
{
    CHECK(this, "this is NULL");

    free(this->name);
    this->name = NULL;
    this->count = 0;
    free(this->verts);
    free(this->norms);
    free(this->txcds);
    this->verts = NULL;
    this->norms = NULL;
    this->txcds = NULL;
    if (this->mat)
    {
        raw_material_term(this->mat);
    }
    free(this->mat);
    this->mat = NULL;

error:;
}

void raw_mesh_generate_cube(raw_mesh_t *this, float size)
{
    int i;
    float cube_verts[] = {
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f
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

    CHECK(this, "this is NULL");

    for (i = 0; i < this->count; ++i)
    {
        raw_mesh_term(&this->meshes[i]);
    }
    free(this->meshes);
    this->meshes = NULL;
    this->count = 0;

error:;
}

bool raw_model_load_from_file(raw_model_t *this, const char *filename, const char *name)
{
    int i = 0;
    char *pch = NULL;

    CHECK(this, "this is NULL");
    CHECK(filename, "filename is NULL");

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

void material_init(material_t *this)
{
    CHECK(this, "this is NULL");

    this->name = NULL;
    vec4f_init(this->ambient, 0.0f);
    this->ambient[3] = 1.0f;
    vec4f_init(this->diffuse, 0.0f);
    this->diffuse[3] = 1.0f;
    vec4f_init(this->specular, 0.0f);
    this->specular[3] = 1.0f;
    this->dissolve = 0.0f;
    this->shininess = 0.0f;
    this->ambient_map = 0;
    this->diffuse_map = 0;
    this->specular_map = 0;
    this->bump_map = 0;
    this->refl_map = 0;

error:;
}

void material_term(material_t *this)
{
    CHECK(this, "this is NULL");

    free(this->name);
    this->name = NULL;
    vec4f_init(this->ambient, 0.0f);
    this->ambient[3] = 1.0f;
    vec4f_init(this->diffuse, 0.0f);
    this->diffuse[3] = 1.0f;
    vec4f_init(this->specular, 0.0f);
    this->specular[3] = 1.0f;
    this->dissolve = 0.0f;
    this->shininess = 0.0f;
    glDeleteTextures(1, &this->ambient_map);
    glDeleteTextures(1, &this->diffuse_map);
    glDeleteTextures(1, &this->specular_map);
    glDeleteTextures(1, &this->bump_map);
    glDeleteTextures(1, &this->refl_map);
    this->ambient_map = 0;
    this->diffuse_map = 0;
    this->specular_map = 0;
    this->bump_map = 0;
    this->refl_map = 0;

error:;
}

bool material_load_from_raw(material_t *this, raw_material_t *raw)
{
    CHECK(this, "this is NULL");
    CHECK(raw, "raw is NULL");

    this->name = _strndup(raw->name, V3D_MAX_NAME_LEN);
    CHECK_MEM(this->name);
    vec4f_copy(this->ambient, raw->ambient);
    vec4f_copy(this->diffuse, raw->diffuse);
    vec4f_copy(this->specular, raw->specular);
    this->dissolve = raw->dissolve;
    this->shininess = raw->shininess;
    if (raw->ambient_map)
    {
        this->ambient_map = load_texture(raw->ambient_map);
    }
    if (raw->diffuse_map)
    {
        this->diffuse_map = load_texture(raw->diffuse_map);
    }
    if (raw->specular_map)
    {
        this->specular_map = load_texture(raw->specular_map);
    }
    if (raw->bump_map)
    {
        this->bump_map = load_texture(raw->bump_map);
    }
    if (raw->refl_map)
    {
        this->refl_map = load_texture(raw->refl_map);
    }

    return true;

error:

    return false;
}

void mesh_init(mesh_t *this)
{
    CHECK(this, "this is NULL");

    this->count = 0;
    this->vao = 0;
    this->mat = NULL;

error:;
}

void mesh_term(mesh_t *this)
{
    CHECK(this, "this is NULL");

    this->count = 0;
    glDeleteVertexArrays(1, &this->vao);
    this->vao = 0;
    free(this->mat);
    this->mat = NULL;

error:;
}

bool mesh_load_from_raw(mesh_t *this, raw_mesh_t *raw)
{
    CHECK(this, "this is NULL");

    this->count = raw->count;
    if (raw->mat)
    {
        this->mat = malloc(sizeof(material_t));
        material_init(this->mat);
        material_load_from_raw(this->mat, raw->mat);
    }

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

error:

    return false;
}

void model_init(model_t *this)
{
    CHECK(this, "this is NULL");

    this->count = 0;
    this->meshes = NULL;
    this->_shader_id = 0;
    this->_shader_data = NULL;

error:;
}

void model_term(model_t *this)
{
    int i;

    CHECK(this, "this is NULL");

    for (i = 0; i < this->count; ++i)
    {
        mesh_term(&this->meshes[i]);
    }
    this->count = 0;
    free(this->meshes);
    this->meshes = NULL;
    this->_shader_id = 0;

error:;
}

bool model_load_from_file(model_t *this, const char *filename, const char *name, GLuint shader_id, shader_data_t *shader_data)
{
    bool ret;
    raw_model_t raw;

    CHECK(this, "this is NULL");
    CHECK(filename, "filename is NULL");

    raw_model_init(&raw);
    raw_model_load_from_file(&raw, filename, name);
    ret = model_load_from_raw(this, &raw, shader_id, shader_data);

    raw_model_term(&raw);

    return ret;

error:

    return false;
}

bool model_load_from_raw(model_t *this, raw_model_t *raw, GLuint shader_id, shader_data_t *shader_data)
{
    int i;

    CHECK(this, "this is NULL");
    CHECK(raw, "raw is NULL");

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

    this->mtl_shininess_loc = glGetUniformLocation(this->_shader_id, "u_mtl_shininess");
    this->mtl_ambient_loc = glGetUniformLocation(this->_shader_id, "u_mtl_ambient");
    this->tex_ambient_loc = glGetUniformLocation(this->_shader_id, "u_tex_ambient");
    this->mtl_diffuse_loc = glGetUniformLocation(this->_shader_id, "u_mtl_diffuse");
    this->tex_diffuse_loc = glGetUniformLocation(this->_shader_id, "u_tex_diffuse");
    this->mtl_specular_loc = glGetUniformLocation(this->_shader_id, "u_mtl_specular");
    this->tex_specular_loc = glGetUniformLocation(this->_shader_id, "u_tex_specular");
    this->tex_bump_loc = glGetUniformLocation(this->_shader_id, "u_tex_bump");

    return true;

error:

    return false;
}

void model_draw(model_t *this)
{
    int i;
    mesh_t *mesh = NULL;
    material_t *mat = NULL;

    CHECK(this, "this is NULL");

    glUseProgram(this->_shader_id);

    if (this->_shader_data)
    {
        shader_data_bind(this->_shader_data);
    }

    for (i = 0; i < this->count; ++i)
    {
        mesh = &this->meshes[i];
        mat = mesh->mat;

        glUniform1i(this->tex_ambient_loc, -1);
        glUniform1i(this->tex_diffuse_loc, -1);
        glUniform1i(this->tex_specular_loc, -1);
        glUniform1i(this->tex_bump_loc, -1);

        if (mat)
        {
            glUniform1f(this->mtl_shininess_loc, mat->shininess);
            glUniform4fv(this->mtl_ambient_loc, 1, mat->ambient);
            glUniform4fv(this->mtl_diffuse_loc, 1, mat->diffuse);
            glUniform4fv(this->mtl_specular_loc, 1, mat->specular);

            if (mat->ambient_map)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mat->ambient_map);
                glUniform1i(this->tex_ambient_loc, 0);
            }

            if (mat->diffuse_map)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mat->diffuse_map);
                glUniform1i(this->tex_diffuse_loc, 1);
            }

            if (mat->specular_map)
            {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, mat->specular_map);
                glUniform1i(this->tex_specular_loc, 2);
            }

            if (mat->bump_map)
            {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, mat->specular_map);
                glUniform1i(this->tex_bump_loc, 3);
            }
        }

        glBindVertexArray(mesh->vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh->count);
    }

error:;
}
