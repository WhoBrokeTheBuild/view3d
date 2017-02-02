#include "model.h"
#include <string.h>

void material_init(material_t *this)
{
  assert(this);

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
}

void material_term(material_t *this)
{
  assert(this);

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
}

bool material_load_from_mdl(material_t *this, mdl_material_t *mdl)
{
  assert(this);
  assert(mdl);

  this->name = _strndup(mdl->name, V3D_MAX_NAME_LEN);
  if (!this->name)
  {
    DEBUG_ERROR("Out of memory");
    goto error;
  }

  vec4f_copy(this->ambient, mdl->ambient);
  vec4f_copy(this->diffuse, mdl->diffuse);
  vec4f_copy(this->specular, mdl->specular);

  this->dissolve = mdl->dissolve;
  this->shininess = mdl->shininess;

  if (mdl->ambient_map)
  {
    this->ambient_map = load_texture(mdl->ambient_map);
  }
  if (mdl->diffuse_map)
  {
    this->diffuse_map = load_texture(mdl->diffuse_map);
  }
  if (mdl->specular_map)
  {
    this->specular_map = load_texture(mdl->specular_map);
  }
  if (mdl->bump_map)
  {
    this->bump_map = load_texture(mdl->bump_map);
  }
  if (mdl->refl_map)
  {
    this->refl_map = load_texture(mdl->refl_map);
  }

  return true;

error:

  return false;
}

void mesh_init(mesh_t *this)
{
  assert(this);

  this->count = 0;
  this->vao = 0;
  this->mat = NULL;
}

void mesh_term(mesh_t *this)
{
  assert(this);

  this->count = 0;
  glDeleteVertexArrays(1, &this->vao);
  this->vao = 0;
  free(this->mat);
  this->mat = NULL;
}

bool mesh_load_from_mdl(mesh_t *this, mdl_mesh_t *mdl)
{
  assert(this);

  bool ret = false;

  this->count = mdl->count;
  if (mdl->mat)
  {
    this->mat = malloc(sizeof(material_t));
    material_init(this->mat);
    ret = material_load_from_mdl(this->mat, mdl->mat);
    if (!ret)
    {
      DEBUG_ERROR("Failed to load material from mdl struct");
      return false;
    }
  }

  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);

  GLuint vbos[4];
  glGenBuffers(4, vbos);

  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mdl->count * 3, mdl->verts,
               GL_STATIC_DRAW);
  glVertexAttribPointer(MODEL_ATTRID_VERTS, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(MODEL_ATTRID_VERTS);

  if (mdl->norms)
  {
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mdl->count * 3, mdl->norms,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(MODEL_ATTRID_NORMS, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(MODEL_ATTRID_NORMS);
  }

  if (mdl->txcds)
  {
    glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mdl->count * 2, mdl->txcds,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(MODEL_ATTRID_TXCDS, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(MODEL_ATTRID_TXCDS);
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return true;
}

void model_init(model_t *this)
{
  assert(this);

  this->count = 0;
  this->meshes = NULL;
  this->_shader_id = 0;
  this->_shader_data = NULL;
}

void model_term(model_t *this)
{
  assert(this);

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
  assert(this);
  assert(filename);

  bool ret;
  mdl_model_t mdl;

  mdl_model_init(&mdl);
  mdl_load_from_file(&mdl, filename, name);
  ret = model_load_from_mdl(this, &mdl, shader_id, shader_data);

  mdl_model_term(&mdl);

  return ret;
}

bool model_load_from_mdl(model_t *this, mdl_model_t *mdl, GLuint shader_id, shader_data_t *shader_data)
{
  assert(this);
  assert(mdl);

  int i, j, k;

  this->_shader_id = shader_id;
  if (shader_data)
  {
    this->_shader_data = malloc(sizeof(shader_data_t));
    memcpy(this->_shader_data, shader_data, sizeof(shader_data_t));
    shader_data_init(this->_shader_data, this->_shader_id);
  }

  vec3f_init(this->min, 0.0f);
  vec3f_init(this->max, 0.0f);
  for (i = 0; i < mdl->count; ++i)
  {
    for (j = 0; j < mdl->meshes[i].count; ++j)
    {
      for (k = 0; k < 3; ++k)
      {
        if (mdl->meshes[i].verts[(j * 3) + k] < this->min[k])
        {
          this->min[k] = mdl->meshes[i].verts[(j * 3) + k];
        }
        if (mdl->meshes[i].verts[(j * 3) + k] > this->max[k])
        {
          this->max[k] = mdl->meshes[i].verts[(j * 3) + k];
        }
      }
    }
  }

  this->count = mdl->count;
  this->meshes = malloc(sizeof(mesh_t) * mdl->count);
  for (i = 0; i < mdl->count; ++i)
  {
    mesh_init(&this->meshes[i]);
    if (!mesh_load_from_mdl(&this->meshes[i], &mdl->meshes[i]))
    {
      DEBUG_ERROR("Failed to load mesh from mdl struct");
      goto error;
    }
  }

  this->mtl_shininess_loc = glGetUniformLocation(this->_shader_id, "u_mtl_shininess");
  this->mtl_ambient_loc = glGetUniformLocation(this->_shader_id, "u_mtl_ambient");
  this->tex_ambient_loc = glGetUniformLocation(this->_shader_id, "u_tex_ambient");
  this->has_tex_ambient_loc = glGetUniformLocation(this->_shader_id, "u_has_tex_ambient");
  this->mtl_diffuse_loc = glGetUniformLocation(this->_shader_id, "u_mtl_diffuse");
  this->tex_diffuse_loc = glGetUniformLocation(this->_shader_id, "u_tex_diffuse");
  this->has_tex_diffuse_loc = glGetUniformLocation(this->_shader_id, "u_has_tex_diffuse");
  this->mtl_specular_loc = glGetUniformLocation(this->_shader_id, "u_mtl_specular");
  this->tex_specular_loc = glGetUniformLocation(this->_shader_id, "u_tex_specular");
  this->has_tex_specular_loc = glGetUniformLocation(this->_shader_id, "u_has_tex_specular");
  this->tex_bump_loc = glGetUniformLocation(this->_shader_id, "u_tex_bump");
  this->has_tex_bump_loc = glGetUniformLocation(this->_shader_id, "u_has_tex_bump");

  return true;

error:

  return false;
}

void model_draw(model_t *this)
{
  assert(this);

  int i;
  mesh_t *mesh = NULL;
  material_t *mat = NULL;

  glUseProgram(this->_shader_id);

  if (this->_shader_data)
  {
    shader_data_bind(this->_shader_data);
  }

  for (i = this->count - 1; i >= 0; --i)
  {
    mesh = &this->meshes[i];
    mat = mesh->mat;

    glUniform1i(this->has_tex_ambient_loc, false);
    glUniform1i(this->has_tex_diffuse_loc, false);
    glUniform1i(this->has_tex_specular_loc, false);
    glUniform1i(this->has_tex_bump_loc, false);

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
        glUniform1i(this->has_tex_ambient_loc, true);
      }

      if (mat->diffuse_map)
      {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mat->diffuse_map);
        glUniform1i(this->tex_diffuse_loc, 1);
        glUniform1i(this->has_tex_diffuse_loc, true);
      }

      if (mat->specular_map)
      {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mat->specular_map);
        glUniform1i(this->tex_specular_loc, 2);
        glUniform1i(this->has_tex_specular_loc, true);
      }

      if (mat->bump_map)
      {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mat->specular_map);
        glUniform1i(this->tex_bump_loc, 3);
        glUniform1i(this->has_tex_bump_loc, true);
      }
    }

    glBindVertexArray(mesh->vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh->count);
  }
}
