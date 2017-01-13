#include <model_obj.h>
#include <stdio.h>

const int OBJ_DEF_ARR_SIZE = 10;

bool raw_model_load_from_obj(raw_model_t *this, const char *filename, const char *name)
{

    int i, j;
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    char *pch;
    vec3f_t tmp;

    int all_verts_cap = OBJ_DEF_ARR_SIZE;
    int all_norms_cap = OBJ_DEF_ARR_SIZE;
    int all_txcds_cap = OBJ_DEF_ARR_SIZE;
    int all_verts_index = 0;
    int all_norms_index = 0;
    int all_txcds_index = 0;
    float *all_verts = malloc(sizeof(float) * all_verts_cap * 3);
    float *all_norms = malloc(sizeof(float) * all_norms_cap * 3);
    float *all_txcds = malloc(sizeof(float) * all_txcds_cap * 2);
    int verts_loaded = 0;
    int norms_loaded = 0;
    int txcds_loaded = 0;

    bool has_norm = false;
    bool has_txcd = false;
    vec3i_t face[3];
    vec3i_init(face[0], 0);
    vec3i_init(face[1], 0);
    vec3i_init(face[2], 0);

    raw_mesh_t *mesh = NULL;
    int mesh_verts_cap = 10;
    int mesh_norms_cap = 10;
    int mesh_txcds_cap = 10;
    int mesh_verts_index = 0;
    int mesh_norms_index = 0;
    int mesh_txcds_index = 0;
    char mesh_name[V3D_MAX_NAME_LEN + 1];
    mesh_name[V3D_MAX_NAME_LEN] = '\0';
    bool read_first_mesh = false;

    raw_model_init(this);
    ++this->count;
    this->meshes = malloc(sizeof(raw_mesh_t) * this->count);
    mesh = &this->meshes[0];
    raw_mesh_init(mesh);

    mesh->verts = malloc(sizeof(float) * mesh_verts_cap * 3);
    mesh->norms = malloc(sizeof(float) * mesh_norms_cap * 3);
    mesh->txcds = malloc(sizeof(float) * mesh_txcds_cap * 2);

    fp = fopen(filename, "r");
    CHECK(fp, "Failed to open '%s'", filename);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (read == 0)
            break;
        if (line[0] == '#' || line[0] == '\n')
            continue;

        if (line[0] == 'v')
        {
            if (line[1] == 'n')
            {
                sscanf(line, "%*s %f %f %f", &tmp[0], &tmp[1], &tmp[2]);
                vec3f_copy(all_norms + (all_norms_index * 3), tmp);
                ++all_norms_index;
            }
            else if (line[1] == 't')
            {
                sscanf(line, "%*s %f %f", &tmp[0], &tmp[1]);
                vec2f_copy(all_txcds + (all_txcds_index * 2), tmp);
                ++all_txcds_index;
            }
            else
            {
                sscanf(line, "%*s %f %f %f", &tmp[0], &tmp[1], &tmp[2]);
                vec3f_copy(all_verts + (all_verts_index * 3), tmp);
                ++all_verts_index;
            }
        }
        else if (line[0] == 'f')
        {
            has_norm = false;
            has_txcd = false;
            vec3i_init(face[0], 0);
            vec3i_init(face[1], 0);
            vec3i_init(face[2], 0);
            pch = strstr(line, "//");
            if (pch)
            {
                has_norm = true;
                sscanf(line, "%*s %d//%d %d//%d %d//%d", &face[0][0], &face[0][2], &face[1][0], &face[1][2], &face[2][0], &face[2][2]);
            }
            else
            {
                pch = strchr(line, '/');
                if (pch)
                {
                    has_norm = true;
                    has_txcd = true;
                    sscanf(line, "%*s %d/%d/%d %d/%d/%d %d/%d/%d", &face[0][0], &face[0][1], &face[0][2], &face[1][0], &face[1][1], &face[1][2], &face[2][0], &face[2][1], &face[2][2]);
                }
                else
                {
                    sscanf(line, "%*s %d %d %d", &face[0][0], &face[1][0], &face[2][0]);
                }
            }

            for (i = 0; i < 3; ++i)
            {
                if (face[i][0] < 0)
                {
                    face[i][0] += all_verts_index;
                }
                if (face[i][1] < 0)
                {
                    face[i][1] += all_txcds_index;
                }
                if (face[i][2] < 0)
                {
                    face[i][2] += all_norms_index;
                }

                vec3f_copy(mesh->verts + (mesh_verts_index * 3), all_verts + ((face[i][0] - 1) * 3));
                ++mesh_verts_index;

                if (has_norm)
                {
                    vec3f_copy(mesh->norms + (mesh_norms_index * 3), all_norms + ((face[i][2] - 1) * 3));
                    ++mesh_norms_index;
                }

                if (has_txcd)
                {
                    vec2f_copy(mesh->txcds + (mesh_txcds_index * 2), all_txcds + ((face[i][1] - 1) * 2));
                    ++mesh_txcds_index;
                }

                ++mesh->count;
            }

            if (mesh_verts_index >= mesh_verts_cap - 6)
            {
                mesh_verts_cap *= 2;
                mesh->verts = realloc(mesh->verts, sizeof(float) * mesh_verts_cap * 3);
            }

            if (mesh_norms_index >= mesh_norms_cap - 6)
            {
                mesh_norms_cap *= 2;
                mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_cap * 3);
            }

            if (mesh_txcds_index >= mesh_txcds_cap - 6)
            {
                mesh_txcds_cap *= 2;
                mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_txcds_cap * 2);
            }
        }
        else if (line[0] == 'g' || line[0] == 'o')
        {
            if (read_first_mesh)
            {
                mesh->verts = realloc(mesh->verts, sizeof(float) * mesh_verts_index * 3);

                if (mesh_norms_index == 0)
                {
                    LOG_INFO("Generating normals for '%s'", mesh->name);
                    mesh_norms_index = mesh_verts_index;
                    mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
                    for (j = 0; j < mesh_verts_index; ++j)
                    {
                        calc_normal(&mesh->norms[j * 3 + 0], &mesh->verts[j * 3 + 0], &mesh->verts[j * 3 + 1], &mesh->verts[j * 3 + 2]);
                        vec3f_copy(&mesh->norms[j * 3 + 1], &mesh->norms[i * 3]);
                        vec3f_copy(&mesh->norms[j * 3 + 2], &mesh->norms[i * 3]);
                    }
                }
                else
                {
                    mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
                }

                if (mesh_txcds_index == 0)
                {
                    free(mesh->txcds);
                    mesh->txcds = 0;
                }
                {
                    mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_txcds_index * 2);
                }

                verts_loaded += mesh_verts_index;
                norms_loaded += mesh_norms_index;
                txcds_loaded += mesh_txcds_index;

                ++this->count;
                this->meshes = realloc(this->meshes, sizeof(raw_mesh_t) * this->count);
                mesh = &this->meshes[this->count - 1];
                raw_mesh_init(mesh);

                mesh_verts_index = 0;
                mesh_norms_index = 0;
                mesh_txcds_index = 0;
                mesh_verts_cap = OBJ_DEF_ARR_SIZE;
                mesh_norms_cap = OBJ_DEF_ARR_SIZE;
                mesh_txcds_cap = OBJ_DEF_ARR_SIZE;
                mesh->verts = malloc(sizeof(float) * mesh_verts_cap * 3);
                mesh->norms = malloc(sizeof(float) * mesh_norms_cap * 3);
                mesh->txcds = malloc(sizeof(float) * mesh_txcds_cap * 2);
            }
            else
            {
                read_first_mesh = true;
            }

            sscanf(line, "%*s %s" V3D_MAX_NAME_LEN_FMT "s", mesh_name);
            mesh->name = _strndup(mesh_name, V3D_MAX_NAME_LEN);
        }

        if (all_verts_index >= all_verts_cap - 1)
        {
            all_verts_cap *= 2;
            all_verts = realloc(all_verts, sizeof(float) * all_verts_cap * 3);
        }

        if (all_norms_index >= all_norms_cap - 1)
        {
            all_norms_cap *= 2;
            all_norms = realloc(all_norms, sizeof(float) * all_norms_cap * 3);
        }

        if (all_txcds_index >= all_txcds_cap - 1)
        {
            all_txcds_cap *= 2;
            all_txcds = realloc(all_txcds, sizeof(float) * all_txcds_cap * 2);
        }
    }

    mesh->verts = realloc(mesh->verts, sizeof(float) * mesh_verts_index * 3);

    if (mesh_norms_index == 0)
    {
        LOG_INFO("Generating normals for '%s'", mesh->name);
        mesh_norms_index = mesh_verts_index;
        mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
        for (j = 0; j < mesh_verts_index; ++j)
        {
            calc_normal(&mesh->norms[j * 3 + 0], &mesh->verts[j * 3 + 0], &mesh->verts[j * 3 + 1], &mesh->verts[j * 3 + 2]);
            vec3f_copy(&mesh->norms[j * 3 + 1], &mesh->norms[i * 3]);
            vec3f_copy(&mesh->norms[j * 3 + 2], &mesh->norms[i * 3]);
        }
    }
    else
    {
        mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_index * 3);
    }

    if (mesh_txcds_index == 0)
    {
        free(mesh->txcds);
        mesh->txcds = 0;
    }
    {
        mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_txcds_index * 2);
    }

    verts_loaded += mesh_verts_index;
    norms_loaded += mesh_norms_index;
    txcds_loaded += mesh_txcds_index;

    free(all_verts);
    free(all_norms);
    free(all_txcds);

    fclose(fp);
    free(line);

    LOG_INFO("Loaded %s: Verts %d, Norms %d, Tex Coords %d\n", filename, verts_loaded, norms_loaded, txcds_loaded);

    return true;

error:

    free(all_verts);
    free(all_norms);
    free(all_txcds);

    fclose(fp);
    free(line);

    return false;
}
