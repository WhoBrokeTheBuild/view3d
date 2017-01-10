#include "model_fbx.h"
#include <stdio.h>
#include <zlib.h>

const size_t FBX_MAX_NODE_NAME_LEN = 512;
const int FBX_HEADER_SIZE = 27;
const char *FBX_FILE_ID = "Kaydara FBX Binary  ";
const char *FBX_IGNORE[] = {
    "FBXHeaderExtension",
    "FileId",
    "CreationTime",
    "Creator",
    "GlobalSettings",
    "Documents",
    "References",
    "Definitions",
    "Connections",
    "Takes",
    NULL
};

bool fbx_inflate_data(char *dst, char *src, size_t dst_len, size_t src_len)
{
    int zstatus = Z_OK;
    z_stream zs;

    memset(&zs, 0, sizeof(zs));
    zs.avail_in = 0;
    zs.next_in = Z_NULL;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;

    zstatus = inflateInit(&zs);
    CHECK(zstatus == Z_OK, "Failed to init zlib stream: %s", zs.msg);

    zs.next_in = (Bytef *)src;
    zs.avail_in = src_len;
    zs.next_out = (Bytef *)dst;
    zs.avail_out = dst_len;

    zstatus = inflate(&zs, Z_FINISH);
    CHECK(zstatus != Z_STREAM_ERROR, "zlib state broken");
    CHECK(zstatus != Z_DATA_ERROR, "Data is corrupt");
    CHECK(zstatus != Z_MEM_ERROR, "Out of memory");
    CHECK(zstatus == Z_STREAM_END, "Failed to inflate data: %s", zs.msg);

    inflateEnd(&zs);
    return true;

error:

    inflateEnd(&zs);
    return false;
}

typedef struct fbx_prop fbx_prop_t;
typedef struct fbx_node fbx_node_t;

typedef size_t (*fbx_prop_read_func_t)(fbx_prop_t *, char *, size_t, size_t);

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
        int *array;

        char *S;
        void *R;

        bool C;
        int16_t Y;
        int32_t I;
        int64_t L;
        float F;
        double D;

        bool *b;
        unsigned char *c;
        int32_t *i;
        int64_t *l;
        float *f;
        double *d;
    };
} fbx_prop_t;

typedef struct fbx_prop_handler
{
    char type;
    fbx_prop_read_func_t func;

} fbx_prop_handler_t;

size_t fbx_prop_read_string(fbx_prop_t *this, char *buffer, size_t offset, size_t buffer_len)
{
    memcpy(&this->length, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    this->S = malloc((this->length + 1) * sizeof(char));
    memcpy(this->S, buffer + offset, this->length);
    this->S[this->length] = '\0';
    offset += this->length;

    return offset;
}

size_t fbx_prop_read_data(fbx_prop_t *this, char *buffer, size_t offset, size_t buffer_len)
{
    memcpy(&this->length, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    this->R = malloc((this->length) * sizeof(char));
    memcpy(this->R, buffer + offset, this->length);
    offset += this->length;

    return offset;
}

#define FBX_PROP_READ_PRIMITIVE(ID, NAME, TYPE)                                                        \
    size_t fbx_prop_read_prim_##NAME(fbx_prop_t *this, char *buffer, size_t offset, size_t buffer_len) \
    {                                                                                                  \
        memcpy(&this->ID, buffer + offset, sizeof(TYPE));                                              \
        return offset + sizeof(TYPE);                                                                  \
    }

#define FBX_PROP_READ_ARRAY(ID, NAME, TYPE)                                                                 \
    size_t fbx_prop_read_arr_##NAME(fbx_prop_t *this, char *buffer, size_t start_offset, size_t buffer_len) \
    {                                                                                                       \
        size_t offset = start_offset;                                                                       \
        uint32_t encoding = 0;                                                                              \
        uint32_t length = 0;                                                                                \
                                                                                                            \
        memcpy(&this->length, buffer + offset, sizeof(uint32_t));                                           \
        offset += sizeof(uint32_t);                                                                         \
                                                                                                            \
        memcpy(&encoding, buffer + offset, sizeof(uint32_t));                                               \
        offset += sizeof(uint32_t);                                                                         \
                                                                                                            \
        memcpy(&length, buffer + offset, sizeof(uint32_t));                                                 \
        offset += sizeof(uint32_t);                                                                         \
                                                                                                            \
        if (this->length == 0)                                                                              \
        {                                                                                                   \
            return offset;                                                                                  \
        }                                                                                                   \
                                                                                                            \
        this->ID = malloc(this->length * sizeof(TYPE));                                                     \
        if (encoding == 1)                                                                                  \
        {                                                                                                   \
            fbx_inflate_data((char *)this->ID, buffer + offset, this->length * sizeof(TYPE), length);       \
        }                                                                                                   \
        else                                                                                                \
        {                                                                                                   \
            memcpy(this->ID, buffer + offset, length);                                                      \
        }                                                                                                   \
        offset += length;                                                                                   \
        return offset;                                                                                      \
    }

FBX_PROP_READ_PRIMITIVE(C, bool, bool);
FBX_PROP_READ_PRIMITIVE(Y, int16, int16_t);
FBX_PROP_READ_PRIMITIVE(I, int32, int32_t);
FBX_PROP_READ_PRIMITIVE(L, int64, int64_t);
FBX_PROP_READ_PRIMITIVE(F, float, float);
FBX_PROP_READ_PRIMITIVE(D, double, double);
FBX_PROP_READ_ARRAY(b, bool, bool);
FBX_PROP_READ_ARRAY(c, byte, unsigned char);
FBX_PROP_READ_ARRAY(i, int32, int32_t);
FBX_PROP_READ_ARRAY(l, int64, int64_t);
FBX_PROP_READ_ARRAY(f, float, float);
FBX_PROP_READ_ARRAY(d, double, double);

fbx_prop_handler_t FBX_PROP_READ_FUNCS[] = {
    { 'S', &fbx_prop_read_string },
    { 'R', &fbx_prop_read_data },
    { 'C', &fbx_prop_read_prim_bool },
    { 'Y', &fbx_prop_read_prim_int16 },
    { 'I', &fbx_prop_read_prim_int32 },
    { 'L', &fbx_prop_read_prim_int64 },
    { 'F', &fbx_prop_read_prim_float },
    { 'D', &fbx_prop_read_prim_double },
    { 'b', &fbx_prop_read_arr_bool },
    { 'c', &fbx_prop_read_arr_byte },
    { 'i', &fbx_prop_read_arr_int32 },
    { 'l', &fbx_prop_read_arr_int64 },
    { 'f', &fbx_prop_read_arr_float },
    { 'd', &fbx_prop_read_arr_double },
    { '\0', NULL }
};

void fbx_prop_init(fbx_prop_t *this)
{
    CHECK(this, "this is NULL");
    memset(this, 0, sizeof(fbx_prop_t));

error:;
}

void fbx_prop_term(fbx_prop_t *this)
{
    CHECK(this, "this is NULL");
    switch (this->type)
    {
    case 'S':
    case 'R':
    case 'b':
    case 'c':
    case 'i':
    case 'l':
    case 'f':
    case 'd':
        free(this->array);
    default:;
    }
    memset(this, 0, sizeof(fbx_prop_t));

error:;
}

size_t fbx_prop_read(fbx_prop_t *this, char *buffer, size_t start_offset, size_t buffer_len)
{
    int i;
    size_t offset = start_offset;

    CHECK(this, "this is NULL");

    memcpy(&this->type, buffer + offset, sizeof(this->type));
    offset += sizeof(this->type);

    for (i = 0; FBX_PROP_READ_FUNCS[i].type; ++i)
    {
        if (FBX_PROP_READ_FUNCS[i].type == this->type)
        {
            return (*FBX_PROP_READ_FUNCS[i].func)(this, buffer, offset, buffer_len);
        }
    }

    LOG_ERR("Unknown property ID '%c'", this->type);

error:

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

    for (i = 0; FBX_IGNORE[i]; ++i)
    {
        if (strcmp(this->name, FBX_IGNORE[i]) == 0)
        {
            offset = this->end_offset;
            fbx_node_term(this);
            return fbx_node_read(this, buffer, offset, buffer_len);
        }
    }

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

    return this->end_offset;

error:

    return offset;
}

fbx_node_t *fbx_node_find(fbx_node_t *parent, const char *in_id)
{
    int i;
    char *id = NULL;
    char *pch = NULL;
    fbx_node_t *tmp = NULL;
    bool multi = false;

    CHECK(parent, "parent is NULL");

    id = strdup(in_id);

    pch = strchr(id, '.');
    if (pch)
    {
        multi = true;
    }

    pch = id;
    if (multi)
    {
        pch = strtok(id, ".");
    }

    for (i = 0; i < parent->num_nodes && pch; ++i)
    {
        if (strcmp(parent->nodes[i].name, pch) == 0)
        {
            if (multi)
            {
                pch = strtok(NULL, ".");
                if (pch == NULL)
                {
                    tmp = &parent->nodes[i];
                    break;
                }
            }
            else
            {
                tmp = &parent->nodes[i];
                break;
            }

            if (parent->nodes[i].num_nodes == 0)
            {
                free(id);
                return NULL;
            }

            i = 0;
            parent = &parent->nodes[i];
            continue;
        }
    }

    free(id);
    return tmp;

error:

    free(id);
    return NULL;
}

bool raw_model_load_from_fbx(raw_model_t *this, const char *filename, const char *name)
{
    int i;
    FILE *fp = NULL;
    char *buffer = NULL;
    size_t buffer_len = 0;
    ssize_t read = 0;
    uint32_t version = 0;
    size_t offset = 0;
    fbx_node_t root_node;
    fbx_node_t *find, *verts, *faces;
    raw_mesh_t *mesh = NULL;
    int mesh_cap = 10;

    fp = fopen(filename, "r");
    CHECK(fp, "Failed to open file '%s'", filename);

    fseek(fp, 0, SEEK_END);
    buffer_len = ftell(fp);
    rewind(fp);

    buffer = (char *)malloc(buffer_len * sizeof(char));
    CHECK(buffer, "Failed to allocate memory for '%s', size %zu", filename, buffer_len);

    read = fread(buffer, buffer_len, sizeof(char), fp);
    CHECK(read > 0, "Failed to read file '%s'", filename);

    fclose(fp);

    CHECK(strncmp(buffer, FBX_FILE_ID, strlen(FBX_FILE_ID)) == 0, "Invalid FBX Format");

    memcpy(&version, buffer + 23, sizeof(unsigned int));
    LOG_INFO("FBX Version %d.%d", version / 1000, version % 1000);

    fbx_node_init(&root_node);
    offset = FBX_HEADER_SIZE;
    for (i = 0; offset < buffer_len; ++i)
    {
        ++root_node.num_nodes;
        root_node.nodes = realloc(root_node.nodes, root_node.num_nodes * sizeof(fbx_node_t));

        fbx_node_init(&root_node.nodes[i]);
        offset = fbx_node_read(&root_node.nodes[i], buffer, offset, buffer_len);
        if (root_node.nodes[i].end_offset == 0)
        {
            fbx_node_term(&root_node.nodes[i]);
            --root_node.num_nodes;
            root_node.nodes = malloc(root_node.num_nodes * sizeof(fbx_node_t));
            break;
        }
    }

    free(buffer);

    raw_model_init(this);
    ++this->count;
    this->meshes = malloc(sizeof(raw_mesh_t) * this->count);
    mesh = &this->meshes[0];
    raw_mesh_init(mesh);

    find = fbx_node_find(&root_node, "Objects.Geometry");
    if (find)
    {
        verts = fbx_node_find(find, "Vertices");
        faces = fbx_node_find(find, "PolygonVertexIndex");
        if (verts && faces)
        {
            fbx_prop_t *face, *vert;
            face = &faces->props[0];
            vert = &verts->props[0];

            int index;
            mesh->verts = malloc(face->length * sizeof(float));
            for (i = 0; i < face->length; ++i)
            {
                index = face->i[i];
                if (index < 0)
                {
                    index += vert->length;
                }
                mesh->verts[i] = vert->d[index];
                ++mesh->count;
            }

            LOG_INFO("Loaded %d verts", mesh->count / 3);
        }
    }

    return true;

error:

    free(buffer);
    fclose(fp);
    return false;
}
