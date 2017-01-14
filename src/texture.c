#include "texture.h"

#define PNG_DEBUG 3
#include <png.h>

void image_init(image_t *this)
{
    this->width = 0;
    this->height = 0;
    this->color_type = 0;
    this->bit_depth = 0;
    this->data = NULL;
}

void image_term(image_t *this)
{
    this->width = 0;
    this->height = 0;
    this->color_type = 0;
    this->bit_depth = 0;
    free(this->data);
    this->data = NULL;
}

bool image_load_png(image_t *this, const char *filename)
{
    int i;
    FILE *fp = NULL;
    png_byte header[8];
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_infop end_info = NULL;
    int num_passes = 0;
    int rowbytes = 0;
    png_bytep *row_pointers = NULL;

    CHECK(this, "this is NULL");
    CHECK(filename, "filename is NULL");

    fp = fopen(filename, "rb");
    CHECK(fp, "Failed to open file '%s'", filename);

    fread(header, 1, 8, fp);
    CHECK(!png_sig_cmp(header, 0, 8), "File '%s' is not a valid PNG file", filename);

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    CHECK(png_ptr, "png_create_read_struct failed");

    info_ptr = png_create_info_struct(png_ptr);
    CHECK(info_ptr, "png_create_info_struct failed");

    end_info = png_create_info_struct(png_ptr);
    CHECK(end_info, "png_create_info_struct failed");

    CHECK(!setjmp(png_jmpbuf(png_ptr)), "Error during init_io");

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    this->width = png_get_image_width(png_ptr, info_ptr);
    this->height = png_get_image_height(png_ptr, info_ptr);
    this->color_type = png_get_color_type(png_ptr, info_ptr);
    this->bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    num_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    CHECK(!setjmp(png_jmpbuf(png_ptr)), "Error during read_image");

    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes - 1) % 4);

    this->data = malloc(rowbytes * this->height * sizeof(png_byte) + 15);
    CHECK_MEM(this->data);

    row_pointers = malloc(this->height * sizeof(png_bytep));
    CHECK_MEM(row_pointers);

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

GLuint load_texture(const char *filename)
{
    GLuint texture = 0;
    image_t img;

    CHECK(filename, "filename is NULL");

    LOG_INFO("Loading texture from '%s'", filename);

    image_init(&img);
    CHECK(image_load_png(&img, filename), "Failed to load '%s'", filename);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    image_term(&img);
    return true;

error:

    image_term(&img);
    return false;
}
