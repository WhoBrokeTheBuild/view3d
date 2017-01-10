#include "model_fbx.h"
#include <stdio.h>
#include <zlib.h>

const size_t FBX_MAX_NODE_NAME_LEN = 512;
const int FBX_HEADER_SIZE = 27;
const char *FBX_FILE_ID = "Kaydara FBX Binary  ";

typedef struct fbx_prop fbx_prop_t;
typedef struct fbx_node fbx_node_t;

void print_depth(int depth)
{
    int i;
    for (i = 0; i < depth; ++i)
        printf("  ");
}

typedef struct fbx_prop
{
    char type;
    uint32_t length;
    union {
        uint64_t prim;

        bool C;
        int16_t Y;
        int32_t I;
        int64_t L;
        float F;
        double D;
    };
    union {
        int *array;

        bool *b;
        unsigned char *c;
        int32_t *i;
        int64_t *l;
        float *f;
        double *d;
    };
    union {
        void *special;

        char *S;
        void *R;
    };
} fbx_prop_t;

void fbx_prop_init(fbx_prop_t *this)
{
    CHECK(this, "this is NULL");

    this->type = '\0';
    this->length = 0;
    this->prim = 0;
    this->array = NULL;
    this->special = NULL;

error:;
}

void fbx_prop_term(fbx_prop_t *this)
{
    CHECK(this, "this is NULL");

    this->type = '\0';
    this->length = 0;
    this->prim = 0;
    free(this->array);
    this->array = NULL;
    free(this->special);
    this->special = NULL;

error:;
}

size_t fbx_prop_read(fbx_prop_t *this, char *buffer, size_t start_offset, size_t buffer_len)
{
    static bool skip = true;
    size_t offset = start_offset;
    uint32_t encoding = 0;
    uint32_t length = 0;
    uint32_t new_length = 0;
    char *source = NULL;
    int zstatus = 0;
    z_stream stream;

    CHECK(this, "this is NULL");

    memcpy(&this->type, buffer + offset, sizeof(this->type));
    offset += sizeof(this->type);

    if (this->type == 'S' || this->type == 'R')
    {
        memcpy(&this->length, buffer + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (this->length == 0)
        {
            return offset;
        }

        if (this->type == 'S')
        {
            this->S = malloc((this->length + 1) * sizeof(char));
            memcpy(this->S, buffer + offset, this->length);
            this->S[this->length] = '\0';
            offset += this->length;
        }
        else if (this->type == 'R')
        {
            this->R = malloc((this->length) * sizeof(char));
            memcpy(this->R, buffer + offset, this->length);
            offset += this->length;
        }
    }
    else if (this->type == 'b' || this->type == 'c' || this->type == 'i' || this->type == 'l' || this->type == 'f' || this->type == 'd')
    {
        memcpy(&this->length, buffer + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        memcpy(&encoding, buffer + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        memcpy(&length, buffer + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (this->length == 0)
        {
            return offset;
        }

        if (encoding == 1)
        {
            LOG_INFO("Preparing to inflate data");

            memset(&stream, 0, sizeof(stream));

            stream.avail_in = 0;
            stream.next_in = Z_NULL;
            stream.zalloc = Z_NULL;
            stream.zfree = Z_NULL;
            stream.opaque = Z_NULL;

            zstatus = inflateInit(&stream);
            CHECK(zstatus == Z_OK, "Failed to init zlib stream %s", stream.msg);

            stream.avail_in = length;
            stream.next_in = (Bytef *)(buffer + offset);

            new_length = length;
            do
            {
                new_length *= 2;
                source = realloc(source, new_length);
                CHECK_MEM(source);

                stream.avail_out = new_length - stream.total_out;
                stream.next_out = (Bytef *)(source + stream.total_out);

                zstatus = inflate(&stream, Z_NO_FLUSH);
                CHECK(zstatus != Z_STREAM_ERROR, "zlib state broken");
                CHECK(zstatus != Z_DATA_ERROR, "Data is corrupt");
                CHECK(zstatus != Z_MEM_ERROR, "Out of memory");
            } while (zstatus != Z_STREAM_END);

            LOG_INFO("Decompressed size %d, was %d. %.2f%% inflation", (int)stream.total_out, length, ((float)stream.total_out / (float)length) * 100.0f);
            length = stream.total_out;
            source = realloc(source, length);
            CHECK_MEM(source);
        }
        else
        {
            source = buffer + offset;
        }

#define TMP_PROP_ARRAY_READ(C, I, T)         \
    case C:                                  \
        if (source)                          \
        {                                    \
            this->I = malloc(length);        \
            memcpy(this->I, source, length); \
            offset += length;                \
        }                                    \
        break;

        switch (this->type)
        {
            TMP_PROP_ARRAY_READ('b', b, bool);
            TMP_PROP_ARRAY_READ('c', c, unsigned char);
            TMP_PROP_ARRAY_READ('i', i, int32_t);
            TMP_PROP_ARRAY_READ('l', l, int64_t);
            TMP_PROP_ARRAY_READ('f', f, float);
            TMP_PROP_ARRAY_READ('d', d, double);
        }

#undef TMP_PROP_ARRAY_READ

        if (encoding == 1)
        {
            printf("Cleaning up from inflation\n");
            free(source);
            source = NULL;
            inflateEnd(&stream);
        }
    }
    else
    {

#define TMP_PROP_PRIM_READ(C, I, T)                   \
    case C:                                           \
        memcpy(&this->I, buffer + offset, sizeof(T)); \
        offset += sizeof(T);                          \
        break;

        switch (this->type)
        {
            TMP_PROP_PRIM_READ('C', C, bool);
            TMP_PROP_PRIM_READ('Y', Y, int16_t);
            TMP_PROP_PRIM_READ('I', I, int32_t);
            TMP_PROP_PRIM_READ('L', L, int64_t);
            TMP_PROP_PRIM_READ('F', F, float);
            TMP_PROP_PRIM_READ('D', D, double);
        default:
            LOG_ERR("Invalid prop type '%c'", this->type);
        }

#undef TMP_PROP_PRIM_READ
    }

    return offset;

error:

    free(source);
    inflateEnd(&stream);

    return offset;
}

void fbx_prop_print(fbx_prop_t *this, int depth)
{
    int i;

    CHECK(this, "this is NULL");

    print_depth(depth);
    printf("Prop[%c]: ", this->type);
    switch (this->type)
    {
    case 'S':
        printf("%d %s", this->length, this->S);
        break;
    case 'R':
        printf("%d // RAW DATA", this->length);
        break;
    case 'C':
        printf("%s", (this->C ? "true" : "false"));
        break;
    case 'Y':
        printf("%d", this->Y);
        break;
    case 'I':
        printf("%d", this->I);
        break;
    case 'L':
        printf("%ld", this->L);
        break;
    case 'F':
        printf("%f", this->F);
        break;
    case 'D':
        printf("%f", this->D);
        break;
    case 'b':
        for (i = 0; this->b && i < this->length; ++i)
        {
            printf("%s, ", (this->b[i] ? "true" : "false"));
        }
        break;
    case 'c':
        for (i = 0; this->c && i < this->length; ++i)
        {
            printf("%u (%c), ", this->c[i], this->c[i]);
        }
        break;
    case 'i':
        for (i = 0; this->i && i < this->length; ++i)
        {
            printf("%d, ", this->i[i]);
        }
        break;
    case 'l':
        for (i = 0; this->l && i < this->length; ++i)
        {
            printf("%ld, ", this->l[i]);
        }
        break;
    case 'f':
        for (i = 0; this->f && i < this->length; ++i)
        {
            printf("%f, ", this->f[i]);
        }
        break;
    case 'd':
        for (i = 0; this->d && i < this->length; ++i)
        {
            printf("%f, ", this->d[i]);
        }
        break;
    }
    printf("\n");

error:;
}

typedef struct fbx_node
{
    uint32_t end_offset;
    uint32_t num_props;
    uint32_t num_nodes;
    char *name;
    fbx_prop_t *props;
    fbx_node_t *nodes;

} fbx_node_t;

void fbx_node_init(fbx_node_t *this)
{
    CHECK(this, "this is NULL");

    this->end_offset = 0;
    this->num_props = 0;
    this->num_nodes = 0;
    this->name = NULL;
    this->props = NULL;
    this->nodes = NULL;

error:;
}

void fbx_node_term(fbx_node_t *this)
{
    int i;

    CHECK(this, "this is NULL");

    this->end_offset = 0;
    free(this->name);
    this->name = NULL;
    for (i = 0; this->props && i < this->num_props; ++i)
    {
        fbx_prop_term(&this->props[i]);
    }
    free(this->props);
    this->props = NULL;
    this->num_props = 0;
    for (i = 0; this->nodes && i < this->num_nodes; ++i)
    {
        fbx_node_term(&this->nodes[i]);
    }
    free(this->nodes);
    this->nodes = NULL;
    this->num_nodes = 0;

error:;
}

void fbx_node_print(fbx_node_t *this, int depth)
{
    int i;

    CHECK(this, "this is NULL");

    print_depth(depth);
    printf("<Node>\n");

    print_depth(depth);
    printf("Name: %s\n", this->name);

    print_depth(depth);
    printf("End Offset: %u\n", this->end_offset);

    print_depth(depth);
    printf("Num Props: %u\n", this->num_props);

    for (i = 0; this->props && i < this->num_props; ++i)
    {
        fbx_prop_print(&this->props[i], depth + 1);
    }

    for (i = 0; this->nodes && i < this->num_nodes; ++i)
    {
        fbx_node_print(&this->nodes[i], depth + 1);
    }

    print_depth(depth);
    printf("</Node>\n");

error:;
}

size_t fbx_node_read(fbx_node_t *this, char *buffer, size_t start_offset, size_t buffer_len)
{
    int i;
    size_t name_len = 0;
    uint32_t prop_list_len = 0;
    size_t offset = start_offset;

    CHECK(this, "this is NULL");

    memcpy(&this->end_offset, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (this->end_offset == 0)
    {
        return 0;
    }

    memcpy(&this->num_props, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&prop_list_len, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&name_len, buffer + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    CHECK(name_len <= FBX_MAX_NODE_NAME_LEN, "Node name length of %zu exceeds limit of %zu", name_len, FBX_MAX_NODE_NAME_LEN);

    this->name = malloc((name_len + 1) * sizeof(char));
    memcpy(this->name, buffer + offset, name_len);
    this->name[name_len] = '\0';
    offset += name_len;

    fbx_node_print(this, 0);

    if (this->num_props > 0)
    {
        this->props = malloc(this->num_props * sizeof(fbx_prop_t));
        CHECK_MEM(this->props);
        for (i = 0; i < this->num_props; ++i)
        {
            fbx_prop_init(&this->props[i]);

            offset = fbx_prop_read(&this->props[i], buffer, offset, buffer_len);
            CHECK(offset > 0, "Failed to read property");
        }
    }

    this->num_nodes = 0;
    this->nodes = NULL;
    for (i = 0; offset < this->end_offset - 13; ++i)
    {
        ++this->num_nodes;
        this->nodes = realloc(this->nodes, this->num_nodes * sizeof(fbx_node_t));
        CHECK_MEM(this->nodes);

        fbx_node_init(&this->nodes[i]);
        offset = fbx_node_read(&this->nodes[i], buffer, offset, buffer_len);
        CHECK(offset > 0, "Failed to read node");
    }

    offset = this->end_offset;

    return offset;

error:

    return offset;
}

bool raw_model_load_from_fbx(raw_model_t *this, const char *filename, const char *name)
{
    FILE *fp = NULL;
    char *buffer = NULL;
    size_t buffer_len = 0;
    ssize_t read = 0;
    uint32_t version = 0;
    size_t offset = 0;
    fbx_node_t node;

    fp = fopen(filename, "r");
    CHECK(fp, "Failed to open file '%s'", filename);

    fseek(fp, 0, SEEK_END);
    buffer_len = ftell(fp);
    rewind(fp);

    buffer = (char *)malloc(buffer_len * sizeof(char));
    CHECK(buffer, "Failed to allocate memory for '%s', size %zu", filename, buffer_len);

    read = fread(buffer, buffer_len, sizeof(char), fp);
    CHECK(read > 0, "Failed to read file '%s'", filename);

    CHECK(strncmp(buffer, FBX_FILE_ID, strlen(FBX_FILE_ID)) == 0, "Invalid FBX Format");

    memcpy(&version, buffer + 23, sizeof(unsigned int));
    LOG_INFO("FBX Version %d.%d", version / 1000, version % 1000);

    for (offset = FBX_HEADER_SIZE; offset < buffer_len;)
    {
        fbx_node_init(&node);
        offset = fbx_node_read(&node, buffer, offset, buffer_len);
        if (node.end_offset == 0)
        {
            break;
        }

        fbx_node_print(&node, 0);
        fbx_node_term(&node);
    }

    fclose(fp);

    return true;

error:

    fclose(fp);

    return false;
}
