#include "model.h"
#include <model_obj.h>

void raw_mesh_init(raw_mesh_t* this)
{
    this->name = NULL;
    this->count = 0;
    this->verts = NULL;
    this->norms = NULL;
    this->txcds = NULL;
    vec4f_init(this->color, 0.0f);
    this->diff_map = NULL;
}

void raw_mesh_term(raw_mesh_t* this)
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

void raw_model_init(raw_model_t* this)
{
    this->count = 0;
    this->meshes = NULL;
}

void raw_model_term(raw_model_t* this)
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

bool raw_model_load_from_file(raw_model_t* this, const char* filename, const char* name)
{
    // Assume OBJ for now
    return raw_model_load_from_obj(this, filename, name);
}

void mesh_init(mesh_t* this)
{
    this->count = 0;
    this->vao = 0;
}

void mesh_term(mesh_t* this)
{
    this->count = 0;
    glDeleteVertexArrays(1, &this->vao);
    this->vao = 0;
}

bool mesh_load_from_raw(mesh_t* this, raw_mesh_t* raw)
{
    this->count = raw->count;

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * raw->count * 3, raw->verts, GL_STATIC_DRAW);

    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    return true;
}

void model_init(model_t* this)
{
    this->count = 0;
    this->meshes = NULL;
    this->_shader_id = 0;
    this->_shader_data = NULL;
}

void model_term(model_t* this)
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

bool model_load_from_file(model_t* this, const char* filename, const char* name, GLuint shader_id, shader_data_t* shader_data)
{
    bool ret;
    raw_model_t raw;

    raw_model_init(&raw);
    raw_model_load_from_file(&raw, filename, name);
    ret = model_load_from_raw(this, &raw, shader_id, shader_data);

    raw_model_term(&raw);

    return ret;
}

bool model_load_from_raw(model_t* this, raw_model_t* raw, GLuint shader_id, shader_data_t* shader_data)
{
    int i;

    this->_shader_id = shader_id;
    this->_shader_data = malloc(sizeof(shader_data_t));
    memcpy(this->_shader_data, shader_data, sizeof(shader_data_t));

    this->count = raw->count;
    this->meshes = malloc(sizeof(mesh_t) * raw->count);
    for (i = 0; i < raw->count; ++i)
    {
        mesh_init(&this->meshes[i]);
        mesh_load_from_raw(&this->meshes[i], &raw->meshes[i]);
    }

    return false;    
}

void model_draw(model_t* this)
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
