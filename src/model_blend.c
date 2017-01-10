#include "model_blend.h"
#include <stdio.h>
#include <string.h>

const char BLEND_PTR_SIZE_32 = '_';
const char BLEND_PTR_SIZE_64 = '-';
const char BLEND_LTL_ENDIAN = 'v';
const char BLEND_BIG_ENDIAN = 'V';

bool raw_model_load_from_blend(raw_model_t *this, const char *filename, const char *name)
{
    LOG_ERR("Unsupported file format");
    return false;

    const int BUFFER_SIZE = 4096;
    const char *FILE_ID = "BLENDER";

    FILE *fp = NULL;
    ssize_t read = 0;
    char ptr_size = '\0';
    char endianness = '\0';
    char version_str[4];
    char buffer[BUFFER_SIZE];
    unsigned short version = 0;

    fp = fopen(filename, "r");
    CHECK(fp, "Failed to open '%s'", filename);

    read = fread(buffer, BUFFER_SIZE, sizeof(char), fp);
    CHECK(read > 0, "Failed to read from file '%s'", filename);

    CHECK(strncmp((const char *)buffer, FILE_ID, strlen(FILE_ID)) == 0, "File is not valid blender format '%s'", filename);

    ptr_size = buffer[7];
    endianness = buffer[8];
    memcpy(version_str, buffer + 9, 3);
    version_str[3] = '\0';

    version = atoi(version_str);

    LOG_INFO("Blender file version %c.%s, %d-bit ptrs, %s endianness",
        version_str[0], version_str + 1,
        (ptr_size == BLEND_PTR_SIZE_32 ? 32 : 64),
        (endianness == BLEND_LTL_ENDIAN ? "little" : "big"));

    fclose(fp);

    return true;

error:

    fclose(fp);

    return false;
}
