#ifndef VIEW3D_TEXTURE_H
#define VIEW3D_TEXTURE_H

#include <view3d.h>

typedef struct image
{
  int width;
  int height;
  unsigned char *data;
} image_t;

void image_init(image_t *this);
void image_term(image_t *this);

bool image_load_bmp(image_t *this, GLenum *format, const char *filename);
bool image_load_tga(image_t *this, GLenum *format, const char *filename);
bool image_load_png(image_t *this, GLenum *format, const char *filename);

GLuint load_texture(const char *filename);

#endif // VIEW3D_TEXTURE_H
