#ifndef VIEW3D_SHADERS_H
#define VIEW3D_SHADERS_H

#include "view3d.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct
{
  GLenum type;
  const char *filename;

} shader_info_t;

typedef struct
{
  void *data;
  void (*init_cb)(void *, GLuint);
  void (*bind_cb)(void *);
} shader_data_t;

void shader_data_init(shader_data_t *this, GLuint shader);
void shader_data_bind(shader_data_t *this);

bool shader_program_print_log(GLuint program);
bool shader_print_log(GLuint shader);

GLuint shader_load(const char *filename, GLenum shader_type);
GLuint shader_program_load(shader_info_t *shaders);

#endif // VIEW3D_SHADERS_H
