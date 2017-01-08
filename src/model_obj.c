#include <model_obj.h>
#include <stdio.h>

bool raw_model_load_from_obj(raw_model_t* this, const char* filename, const char* name)
{
    int i;
    FILE* fp = NULL;
    char* line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    char* pch;
    vec3f_t tmp;

    int size = 10;
    int vert_index = 0;
    int norm_index = 0;
    int txcd_index = 0;
    float* all_verts = malloc(sizeof(float) * size * 3);
    float* all_norms = malloc(sizeof(float) * size * 3);
    float* all_txcds = malloc(sizeof(float) * size * 2);
    vec3i_t face[3];
    vec3i_init(face[0], 0);
    vec3i_init(face[1], 0);
    vec3i_init(face[2], 0);
    
#define MAX_MESH_NAME_LEN 100
#define MAX_MESH_NAME_LEN_FMT "99"

    raw_mesh_t* mesh = NULL;
    int mesh_cap = 10;
    char mesh_name[MAX_MESH_NAME_LEN];
    bool read_first_mesh = false;

    raw_model_init(this);
    ++this->count;
    this->meshes = malloc(sizeof(raw_mesh_t) * this->count);
    mesh = &this->meshes[this->count - 1];
    raw_mesh_init(mesh);
    
    mesh->verts = malloc(sizeof(float) * mesh_cap * 3);
    mesh->norms = malloc(sizeof(float) * mesh_cap * 3);
    mesh->txcds = malloc(sizeof(float) * mesh_cap * 2);

    fp = fopen(filename, "r");

    CHECK(fp, "Failed to open %s", filename);

    while ((read = getline(&line, &len, fp)) != -1) 
    {
        if (read == 0) break;
        if (line[0] == '#' || line[0] == '\n') continue;

        if (line[0] == 'v')
        {
            if (line[1] == 'n')
            {
                sscanf(line, "%*s %f %f %f", &tmp[0], &tmp[1], &tmp[2]);
                vec3f_copy(all_norms + (norm_index * 3), tmp);
                ++norm_index;
            }
            else if (line[1] == 't')
            {
                sscanf(line, "%*s %f %f", &tmp[0], &tmp[1]);
                vec2f_copy(all_txcds + (txcd_index * 2), tmp);
                ++txcd_index;
            }
            else
            {
                sscanf(line, "%*s %f %f %f", &tmp[0], &tmp[1], &tmp[2]);
                vec3f_copy(all_verts + (vert_index * 3), tmp);
                ++vert_index;
            }
        }
        else if (line[0] == 'f')
        {
            pch = strstr(line, "//");
            if (pch)
            {
                face[0][1] = 0;
                face[1][1] = 0;
                face[2][1] = 0;
                sscanf(line, "%*s %d//%d %d//%d %d//%d", &face[0][0], &face[0][2], &face[1][0], &face[1][2], &face[2][0], &face[2][0]);
            }
            else 
            {
                sscanf(line, "%*s %d/%d/%d %d/%d/%d %d/%d/%d", &face[0][0], &face[0][1], &face[0][2], &face[1][0], &face[1][1], &face[1][2], &face[2][0], &face[2][1], &face[2][2]);
            }

            for (i = 0; i < 3; ++i)
            {
                if (face[i][0] < 0)
                {
                    face[i][0] += vert_index;
                }
                if (face[i][1] < 0)
                {
                    face[i][1] += txcd_index;
                }
                if (face[i][2] < 0)
                {
                    face[i][2] += norm_index;
                }

                vec3f_copy(mesh->verts + (mesh->count * 3), all_verts + (face[i][0] * 3));
                vec3f_copy(mesh->norms + (mesh->count * 3), all_norms + (face[i][2] * 3));
                vec2f_copy(mesh->txcds + (mesh->count * 2), all_txcds + (face[i][1] * 2));

                ++mesh->count;
            }
            
            if (mesh->count >= mesh_cap - 3)
            {
                mesh_cap *= 2;
                mesh->verts = realloc(mesh->verts, sizeof(float) * mesh_cap * 3);
                mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_cap * 3);
                mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_cap * 2);
            }
        }
        else if (line[0] == 'g' || line[0] == 'o')
        {
            if (read_first_mesh)
            {
                mesh->verts = realloc(mesh->verts, sizeof(float) * mesh->count * 3);
                mesh->norms = realloc(mesh->norms, sizeof(float) * mesh->count * 3);
                mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh->count * 2);

                mesh_cap = 10;
                ++this->count;
                this->meshes = realloc(this->meshes, sizeof(raw_mesh_t) * this->count);
                mesh = &this->meshes[this->count - 1];
                raw_mesh_init(mesh);
                
                mesh->verts = realloc(mesh->verts, sizeof(float) * mesh_cap * 3);
                mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_cap * 3);
                mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_cap * 2);
            }
            else 
            {
                read_first_mesh = true;
            }
            
            sscanf(line, "%*s %" MAX_MESH_NAME_LEN_FMT "s", mesh_name);
            mesh->name = strndup(mesh_name, MAX_MESH_NAME_LEN);
        }

        if (vert_index >= size - 1 || 
            norm_index >= size - 1 ||
            txcd_index >= size - 1)
        {
            size *= 2;
            all_verts = realloc(all_verts, sizeof(float) * size * 3);
            all_norms = realloc(all_norms, sizeof(float) * size * 3);
            all_txcds = realloc(all_txcds, sizeof(float) * size * 2);
        }
    }

    LOG_INFO("Loaded %s: Verts %d, Norms %d, Tex Coords %d\n", filename, vert_index, norm_index, txcd_index);

    free(all_verts);
    free(all_norms);
    free(all_txcds);
    
    mesh->verts = realloc(mesh->verts, sizeof(float) * mesh->count * 3);
    mesh->norms = realloc(mesh->norms, sizeof(float) * mesh->count * 3);
    mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh->count * 2);
   
    fclose(fp);
    free(line);

    return true;

error:

    free(all_verts);
    free(all_norms);
    free(all_txcds);

    fclose(fp);
    free(line);

    return false;
}
