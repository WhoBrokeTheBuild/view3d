#ifndef VIEW3D_SHADERS_H
#define VIEW3D_SHADERS_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_SHADER_COUNT 100
#define MAX_SHADER_SIZE 1000000
#define MAX_SHADER_LOG_SIZE 100000
#define MAX_PROGRAM_LOG_SIZE MAX_SHADER_LOG_SIZE

typedef struct
{
    GLenum type;
    const char *filename;

} shader_info_t;

typedef struct
{
    void* data;
    void(*bind_cb)(void*);
} shader_data_t;

void shader_data_bind(shader_data_t* this);

bool shader_program_print_log(GLuint program);
bool shader_print_log(GLuint shader);

GLuint shader_load(const char *filename, GLenum shader_type);
GLuint shader_program_load(shader_info_t *shaders);

#endif // VIEW3D_SHADERS_H
