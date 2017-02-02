#include "texture.h"
#include <util.h>

#define PNG_DEBUG 3
#include <png.h>

void image_init(image_t *this)
{
  this->width = 0;
  this->height = 0;
  this->data = NULL;
}

void image_term(image_t *this)
{
  this->width = 0;
  this->height = 0;
  free(this->data);
  this->data = NULL;
}

bool image_load_bmp(image_t *this, GLenum *format, const char *filename)
{
  assert(this);
  assert(filename);

  FILE *fp = NULL;

  fp = fopen(filename, "rb");
  if (!fp)
  {
    DEBUG_ERROR("Failed to open file '%s'", filename);
    goto error;
  }

  fclose(fp);

  return true;

error:

  fclose(fp);

  return false;
}

bool image_load_tga(image_t *this, GLenum *format, const char *filename)
{
  enum
  {
    _HEADER_LEN = 18
  };

  assert(this);
  assert(filename);

  int i;
  FILE *fp = NULL;
  uint8_t *buffer = NULL;
  size_t buffer_len = 0;
  ssize_t read = 0;
  size_t offset = 0;
  uint8_t header[_HEADER_LEN];
  int pixel_ind = 0;
  int bytes_per_pixel = 0;
  uint8_t chunk_header = 0;
  static uint8_t decomp[] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static uint8_t iscomp[] = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  memset(header, 0, _HEADER_LEN);

  fp = fopen(filename, "rb");
  if (!fp)
  {
    DEBUG_ERROR("Failed to open file '%s'", filename);
    goto error;
  }

  read = fread(header, _HEADER_LEN, 1, fp);
  if (read == 0)
  {
    DEBUG_ERROR("Failed to read file '%s'", filename);
    goto error;
  }

  bytes_per_pixel = header[16] / 8;
  if (bytes_per_pixel != 3 && bytes_per_pixel != 4)
  {
    DEBUG_ERROR("Unsupported TGA format for '%s', not 24 or 32 bit", filename);
    goto error;
  }

  this->width = header[13] * 256 + header[12];
  this->height = header[15] * 256 + header[14];
  this->data = malloc(this->width * this->height * bytes_per_pixel);

  buffer_len = fsize(fp) - _HEADER_LEN;
  buffer = (uint8_t *)malloc(buffer_len);
  if (!buffer)
  {
    DEBUG_ERROR("Out of memory");
    goto error;
  }

  read = fread(buffer, buffer_len, 1, fp);
  if (read == 0)
  {
    DEBUG_ERROR("Failed to read file '%s'", filename);
    goto error;
  }

  *format = (bytes_per_pixel == 3 ? GL_BGR : GL_BGRA);

  if (memcmp(decomp, header, sizeof(decomp)) == 0)
  {
    memcpy(this->data, buffer, this->width * this->height * bytes_per_pixel);
  }
  else if (memcmp(iscomp, header, sizeof(iscomp)) == 0)
  {
    offset = 0;
    for (pixel_ind = 0; pixel_ind < this->width * this->height;)
    {
      chunk_header = buffer[offset++];
      if (chunk_header < 128)
      {
        ++chunk_header;
        for (i = 0; i < chunk_header; ++i)
        {
          memcpy(this->data + pixel_ind * bytes_per_pixel, buffer + offset,
                 bytes_per_pixel);
          offset += bytes_per_pixel;
          ++pixel_ind;
        }
      }
      else
      {
        chunk_header -= 127;
        for (i = 0; i < chunk_header; ++i)
        {
          memcpy(this->data + pixel_ind * bytes_per_pixel, buffer + offset,
                 bytes_per_pixel);
          ++pixel_ind;
        }
        offset += bytes_per_pixel;
      }
    }
  }
  else
  {
    DEBUG_ERROR("Invalid TGA format for '%s'", filename);
    goto error;
  }

  free(buffer);
  fclose(fp);

  return true;

error:

  free(buffer);
  fclose(fp);

  return false;
}

bool image_load_png(image_t *this, GLenum *format, const char *filename)
{
  enum
  {
    _HEADER_LEN = 8
  };

  assert(this);
  assert(filename);

  int i;
  FILE *fp = NULL;
  png_byte header[8];
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_infop end_info = NULL;
  int bit_depth = 0;
  int color_type = 0;
  int rowbytes = 0;
  png_bytep *row_pointers = NULL;

  fp = fopen(filename, "rb");
  if (!fp)
  {
    DEBUG_ERROR("Failed to open file '%s'", filename);
    goto error;
  }

  fread(header, 1, sizeof(header), fp);
  if (png_sig_cmp(header, 0, sizeof(header)))
  {
    DEBUG_ERROR("File '%s' is not a valid PNG file", filename);
    goto error;
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
  {
    DEBUG_ERROR("png_create_read_struct failed");
    goto error;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    DEBUG_ERROR("png_create_info_struct failed");
    goto error;
  }

  end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    DEBUG_ERROR("png_create_info_struct failed");
    goto error;
  }

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    DEBUG_ERROR("Error during init_io");
    goto error;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, sizeof(header));

  png_read_info(png_ptr, info_ptr);

  this->width = png_get_image_width(png_ptr, info_ptr);
  this->height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  if (color_type == PNG_COLOR_TYPE_RGB)
  {
    *format = GL_RGB;
  }
  else if (color_type == PNG_COLOR_TYPE_RGBA)
  {
    *format = GL_RGBA;
  }
  else
  {
    DEBUG_ERROR("Invalid PNG format for '%s'", filename);
  }

  // num_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    DEBUG_ERROR("Error during read_image");
  }

  rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  // glTexImage2d requires rows to be 4-byte aligned
  rowbytes += 3 - ((rowbytes - 1) % 4);

  this->data = malloc(rowbytes * this->height * sizeof(png_byte));
  if (!this->data)
  {
    DEBUG_ERROR("Out of memory");
    goto error;
  }

  row_pointers = malloc(this->height * sizeof(png_bytep));
  if (!row_pointers)
  {
    DEBUG_ERROR("Out of memory");
    goto error;
  }

  for (i = 0; i < this->height; ++i)
  {
    row_pointers[this->height - 1 - i] = this->data + i * rowbytes;
  }

  png_read_image(png_ptr, row_pointers);

  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  free(row_pointers);
  fclose(fp);

  return true;

error:

  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  free(row_pointers);
  fclose(fp);

  return false;
}

typedef struct image_loader
{
  const char *ext;
  bool (*func)(image_t *, GLenum *, const char *);

} image_loader_t;

image_loader_t g_image_loaders[] = {{".bmp", &image_load_bmp},
                                    {".tga", &image_load_tga},
                                    {".png", &image_load_png},
                                    {NULL, NULL}};

GLuint load_texture(const char *filename)
{
  return 0;

  int i;
  image_t img;
  bool ret = false;
  char *pch = NULL;
  GLuint texture = 0;
  GLenum format = GL_RGB;

  assert(filename != NULL);

  image_init(&img);

  pch = strrchr(filename, '.');
  if (!pch)
  {
    DEBUG_ERROR("No file extension found for '%s'", filename);
    goto error;
  }

  for (i = 0; g_image_loaders[i].ext; ++i)
  {
    if (strcmp(pch, g_image_loaders[i].ext) == 0)
    {
      ret = (*g_image_loaders[i].func)(&img, &format, filename);
      if (!ret)
      {
        DEBUG_ERROR("Failed to load '%s'", filename);
        goto error;
      }
      break;
    }
  }

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, format,
               GL_UNSIGNED_BYTE, img.data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  image_term(&img);
  return texture;

error:

  image_term(&img);
  return 0;
}
