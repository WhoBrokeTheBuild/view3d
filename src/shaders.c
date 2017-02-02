#include "shaders.h"

#include "debug.h"
#include "util.h"

void shader_data_init(shader_data_t *this, GLuint shader) { this->init_cb(this->data, shader); }

void shader_data_bind(shader_data_t *this) { this->bind_cb(this->data); }

bool shader_program_print_log(GLuint program)
{
  enum
  {
    _MAX_LOG_SIZE = 100000
  };

  char *program_log = NULL;
  GLint log_size = 0;
  GLint ret_size = 0;

  if (!glIsProgram(program))
  {
    DEBUG_ERROR("Cannot print program log, %d is not a program", program);
    goto error;
  }

  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
  if (log_size > _MAX_LOG_SIZE)
  {
    DEBUG_ERROR("Program log for %d exceeds maximum size allowed (%d > %d)", program, log_size, _MAX_LOG_SIZE);
    goto error;
  }

  program_log = malloc((log_size + 1) * sizeof(char));
  if (!program_log)
  {
    DEBUG_ERROR("Out of memory");
    goto error;
  }

  glGetProgramInfoLog(program, log_size, &ret_size, program_log);
  if (ret_size <= 0)
  {
    DEBUG_ERROR("Failed to get program log for %d", program);
    goto error;
  }
  program_log[log_size] = '\0';

  DEBUG_INFO("Log for program %d:\n%s", program, program_log);

  free(program_log);
  return true;

error:

  free(program_log);
  return false;
}

bool shader_print_log(GLuint shader)
{
  enum
  {
    _MAX_LOG_SIZE = 100000
  };

  char *shader_log = NULL;
  GLint log_size = 0;
  GLint ret_size = 0;

  if (!glIsShader(shader))
  {
    DEBUG_ERROR("Cannot print shader log, %d is not a shader", shader);
    goto error;
  }

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
  if (log_size > _MAX_LOG_SIZE)
  {
    DEBUG_ERROR("Shader log for %d exceeds maximum size allowed (%d > %d)", shader, log_size, _MAX_LOG_SIZE);
    goto error;
  }

  shader_log = malloc((log_size + 1) * sizeof(char));
  if (!shader_log)
  {
    DEBUG_ERROR("Out of memory");
    goto error;
  }

  glGetShaderInfoLog(shader, log_size, &ret_size, shader_log);
  if (ret_size <= 0)
  {
    DEBUG_ERROR("Failed to get shader log for %d", shader);
    goto error;
  }
  shader_log[log_size] = '\0';

  DEBUG_INFO("Log for shader %d:\n%s", shader, shader_log);

  free(shader_log);
  return true;

error:

  free(shader_log);
  return false;
}

GLuint shader_load(const char *filename, GLenum shader_type)
{
  enum
  {
    _MAX_SHADER_SIZE = 1000000
  };

  GLuint shader = 0;
  char *shader_str = NULL;
  FILE *fp = NULL;
  size_t bytes_read = 0;
  size_t file_size = 0;

  DEBUG_INFO("Loading Shader from '%s'", filename);

  fp = fopen(filename, "rb");
  if (!fp)
  {
    DEBUG_ERROR("Failed to open shader '%s'", filename);
    goto error;
  }

  file_size = fsize(fp);
  if (file_size > _MAX_SHADER_SIZE)
  {
    DEBUG_ERROR("Shader exceeds maximum size allowed (%ld > %d)", file_size, _MAX_SHADER_SIZE);
    goto error;
  }

  shader_str = malloc((file_size + 1) * sizeof(char));
  if (!shader_str)
  {
    DEBUG_ERROR("Out of memory");
    goto error;
  }

  bytes_read = fread(shader_str, 1, file_size, fp);
  if (bytes_read != file_size)
  {
    DEBUG_ERROR("Failed to read whole shader file '%s' read %zu/%ld", filename, bytes_read, file_size);
    goto error;
  }
  shader_str[file_size] = '\0';

  shader = glCreateShader(shader_type);
  if (shader == 0)
  {
    DEBUG_ERROR("Failed to create shader object");
    goto error;
  }
  if (shader == GL_INVALID_ENUM)
  {
    DEBUG_ERROR("Invalid shader type %d", shader_type);
    goto error;
  }

  glShaderSource(shader, 1, (const GLchar **)&shader_str, NULL);
  glCompileShader(shader);

  GLint shader_compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled);
  if (!shader_compiled)
  {
    DEBUG_ERROR("Failed to compile shader '%s'", filename);
    goto error;
  }

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
  enum
  {
    _MAX_SHADER_COUNT = 100
  };

  GLuint program = 0;
  GLuint shader_ids[_MAX_SHADER_COUNT];

  program = glCreateProgram();
  if (program == 0)
  {
    DEBUG_ERROR("Failed to create shader program object");
    goto error;
  }

  for (int i = 0; shaders[i].filename; ++i)
  {
    if (i > _MAX_SHADER_COUNT)
    {
      DEBUG_ERROR("Exceeded maximum number of shaders allowed %d", _MAX_SHADER_COUNT);
      goto error;
    }

    shader_ids[i] = shader_load(shaders[i].filename, shaders[i].type);
    if (shader_ids[i] == -1)
    {
      DEBUG_ERROR("Failed to load shader program");
      goto error;
    }

    glAttachShader(program, shader_ids[i]);
  }

  glLinkProgram(program);

  GLint program_linked = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
  if (!program_linked)
  {
    DEBUG_ERROR("Failed to link shader program");
    goto error;
  }

  return program;

error:

  if (program)
  {
    shader_program_print_log(program);
  }

  for (int i = 0; i < _MAX_SHADER_COUNT; ++i)
  {
    glDeleteShader(shader_ids[i]);
  }
  glDeleteProgram(program);

  return -1;
}
