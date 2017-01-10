#include "shaders.h"

#include <debug.h>
#include <util.h>

void shader_data_bind(shader_data_t *this)
{
    this->bind_cb(this->data);
}

bool shader_program_print_log(GLuint program)
{
    char *program_log = NULL;

    CHECK(glIsProgram(program), "Cannot print program log, %d is not a program", program);

    GLint log_size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
    CHECK(log_size < MAX_PROGRAM_LOG_SIZE,
        "Program log for %d exceeds maximum size allowed (%d > %d)", program, log_size,
        MAX_PROGRAM_LOG_SIZE);

    program_log = malloc((log_size + 1) * sizeof(char));
    CHECK_MEM(program_log);

    int ret_size = 0;
    glGetProgramInfoLog(program, log_size, &ret_size, program_log);
    CHECK(ret_size > 0, "Failed to get program log for %d", program);
    program_log[log_size] = '\0';

    LOG_INFO("Log for program %d:\n%s", program, program_log);

    free(program_log);
    return true;

error:

    free(program_log);
    return false;
}

bool shader_print_log(GLuint shader)
{
    char *shader_log = NULL;

    CHECK(glIsShader(shader), "Cannot print shader log, %d is not a shader", shader);

    GLint log_size = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
    CHECK(log_size < MAX_SHADER_LOG_SIZE,
        "Shader log for %d exceeds maximum size allowed (%d > %d)", shader, log_size,
        MAX_SHADER_LOG_SIZE);

    shader_log = malloc((log_size + 1) * sizeof(char));
    CHECK_MEM(shader_log);

    int ret_size = 0;
    glGetShaderInfoLog(shader, log_size, &ret_size, shader_log);
    CHECK(ret_size > 0, "Failed to get shader log for %d", shader);
    shader_log[log_size] = '\0';

    LOG_INFO("Log for shader %d:\n%s", shader, shader_log);

    free(shader_log);
    return true;

error:

    free(shader_log);
    return false;
}

GLuint shader_load(const char *filename, GLenum shader_type)
{
    GLuint shader = 0;
    char *shader_str = NULL;
    FILE *fp = NULL;

    LOG_INFO("Loading Shader from '%s'", filename);

    fp = fopen(filename, "r");
    CHECK(fp, "Failed to open shader '%s'", filename);

    long int file_size = fsize(fp);
    CHECK(file_size < MAX_SHADER_SIZE, "Shader exceeds maximum size allowed (%ld > %d)", file_size,
        MAX_SHADER_SIZE);

    shader_str = malloc((file_size + 1) * sizeof(char));
    CHECK_MEM(shader_str);

    size_t bytes_read = fread(shader_str, 1, file_size, fp);
    CHECK(bytes_read == file_size, "Failed to read whole shader file '%s' read %zu/%ld", filename,
        bytes_read, file_size);
    shader_str[file_size] = '\0';

    shader = glCreateShader(shader_type);
    CHECK(shader != 0, "Failed to create shader object");
    CHECK(shader != GL_INVALID_ENUM, "Invalid shader type %d", shader_type);

    glShaderSource(shader, 1, (const GLchar **)&shader_str, NULL);
    glCompileShader(shader);

    GLint shader_compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled);
    CHECK(shader_compiled, "Failed to compile shader '%s'", filename);

    free(shader_str);
    fclose(fp);

    return shader;

error:

    if (shader)
    {
        shader_print_log(shader);
    }

    free(shader_str);
    fclose(fp);
    glDeleteShader(shader);

    return -1;
}

GLuint shader_program_load(shader_info_t *shaders)
{
    GLuint program = 0;
    GLuint shader_ids[MAX_SHADER_COUNT];

    program = glCreateProgram();
    CHECK(program != 0, "Failed to create shader program object");

    for (int i = 0; shaders[i].filename; ++i)
    {
        CHECK(i < MAX_SHADER_COUNT, "Exceeded maximum number of shaders allowed %d",
            MAX_SHADER_COUNT);

        shader_ids[i] = shader_load(shaders[i].filename, shaders[i].type);
        CHECK(shader_ids[i] != -1, "Failed to load shader program");

        glAttachShader(program, shader_ids[i]);
    }

    glLinkProgram(program);

    GLint program_linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
    CHECK(program_linked, "Failed to link program");

    return program;

error:

    if (program)
    {
        shader_program_print_log(program);
    }

    for (int i = 0; i < MAX_SHADER_COUNT; ++i)
    {
        glDeleteShader(shader_ids[i]);
    }
    glDeleteProgram(program);

    return -1;
}
