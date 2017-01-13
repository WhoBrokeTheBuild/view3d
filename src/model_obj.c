#include <model_obj.h>
#include <stdio.h>

const int OBJ_DEF_ARR_SIZE = 10;

int raw_material_load_from_mtl(raw_material_t **materials, const char *dir, const char *filename, int count)
{
    char path[V3D_MAX_PATH_LEN];
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    raw_material_t *mat = NULL;

    if (dir)
    {
        strcpy(path, dir);
    }

    strcpy(path + strlen(dir), filename);

    fp = fopen(path, "r");
    CHECK(fp, "Failed to open '%s'", path);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (read == 0)
            break;
        if (line[0] == '#' || line[0] == '\n')
            continue;

        line[read - 1] = '\0';

        if (mat)
        {
            if (strncmp(line, "Kd", 2) == 0)
            {
                sscanf(line, "%*s %f %f %f", &mat->diffuse[0], &mat->diffuse[1], &mat->diffuse[2]);
            }
            else if (strncmp(line, "map_Kd", 6) == 0)
            {
                if (dir)
                {
                    strcpy(path, dir);
                }
                strcpy(path + strlen(dir), line + 7);
                mat->diffuse_map = _strndup(path, V3D_MAX_PATH_LEN);
            }
        }

        if (strncmp(line, "newmtl", 6) == 0)
        {
            ++count;
            *materials = realloc(*materials, count * sizeof(raw_material_t));
            CHECK_MEM(*materials);
            mat = &(*materials)[count - 1];
            raw_material_init(mat);

            mat->name = _strndup(line + 7, V3D_MAX_NAME_LEN);
        }
    }

    fclose(fp);

    return count;

error:

    fclose(fp);

    return count;
}

bool raw_model_load_from_obj(raw_model_t *this, const char *filename, const char *name)
{
    int i, j;
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    char *dir = NULL;
    char *pch = NULL;
    vec3f_t tmp;

    int all_verts_cap = OBJ_DEF_ARR_SIZE;
    int all_norms_cap = OBJ_DEF_ARR_SIZE;
    int all_txcds_cap = OBJ_DEF_ARR_SIZE;
    float *all_verts = NULL;
    float *all_norms = NULL;
    float *all_txcds = NULL;
    int all_verts_index = 0;
    int all_norms_index = 0;
    int all_txcds_index = 0;

    int verts_loaded = 0;
    int norms_loaded = 0;
    int txcds_loaded = 0;

    unsigned int slash_count = 0;
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

    int material_count = 0;
    raw_material_t *materials = NULL;

    all_verts = malloc(sizeof(float) * all_verts_cap * 3);
    all_norms = malloc(sizeof(float) * all_norms_cap * 3);
    all_txcds = malloc(sizeof(float) * all_txcds_cap * 2);
    CHECK_MEM(all_verts);
    CHECK_MEM(all_norms);
    CHECK_MEM(all_txcds);

    raw_model_init(this);
    ++this->count;
    this->meshes = malloc(sizeof(raw_mesh_t) * this->count);
    CHECK_MEM(this->meshes);
    mesh = &this->meshes[0];
    raw_mesh_init(mesh);

    mesh->verts = malloc(sizeof(float) * mesh_verts_cap * 3);
    mesh->norms = malloc(sizeof(float) * mesh_norms_cap * 3);
    mesh->txcds = malloc(sizeof(float) * mesh_txcds_cap * 2);
    CHECK_MEM(mesh->verts);
    CHECK_MEM(mesh->norms);
    CHECK_MEM(mesh->txcds);

    pch = strrchr(filename, '/');
    if (pch)
    {
        dir = _strndup(filename, pch - filename + 1);
    }

    fp = fopen(filename, "r");
    CHECK(fp, "Failed to open '%s'", filename);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (read == 0)
            break;
        if (line[0] == '#' || line[0] == '\n')
            continue;

        line[read - 1] = '\0';

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
                    slash_count = strcntchr(line, '/');
                    if (slash_count > 3)
                    {
                        has_norm = true;
                        has_txcd = true;
                        sscanf(line, "%*s %d/%d/%d %d/%d/%d %d/%d/%d", &face[0][0], &face[0][1], &face[0][2], &face[1][0], &face[1][1], &face[1][2], &face[2][0], &face[2][1], &face[2][2]);
                    }
                    else
                    {
                        has_txcd = true;
                        sscanf(line, "%*s %d/%d %d/%d %d/%d", &face[0][0], &face[0][2], &face[1][0], &face[1][2], &face[2][0], &face[2][2]);
                    }
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
                CHECK_MEM(mesh->verts);
            }

            if (mesh_norms_index >= mesh_norms_cap - 6)
            {
                mesh_norms_cap *= 2;
                mesh->norms = realloc(mesh->norms, sizeof(float) * mesh_norms_cap * 3);
                CHECK_MEM(mesh->norms);
            }

            if (mesh_txcds_index >= mesh_txcds_cap - 6)
            {
                mesh_txcds_cap *= 2;
                mesh->txcds = realloc(mesh->txcds, sizeof(float) * mesh_txcds_cap * 2);
                CHECK_MEM(mesh->txcds);
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
                    CHECK_MEM(mesh->norms);
                    for (j = 0; j < mesh_verts_index; ++j)
                    {
                        calc_normal(&mesh->norms[j * 3 + 0], &mesh->verts[j * 3 + 0], &mesh->verts[j * 3 + 1], &mesh->verts[j * 3 + 2]);
                        vec3f_copy(&mesh->norms[j * 3 + 1], &mesh->norms[j * 3]);
                        vec3f_copy(&mesh->norms[j * 3 + 2], &mesh->norms[j * 3]);
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
                CHECK_MEM(this->meshes);
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
                CHECK_MEM(mesh->verts);
                CHECK_MEM(mesh->norms);
                CHECK_MEM(mesh->txcds);
            }
            else
            {
                read_first_mesh = true;
            }

            sscanf(line, "%*s %s" V3D_MAX_NAME_LEN_FMT "s", mesh_name);
            mesh->name = _strndup(mesh_name, V3D_MAX_NAME_LEN);
        }
        else if (strncmp(line, "mtllib", 6) == 0)
        {
            material_count += raw_material_load_from_mtl(&materials, dir, line + 7, material_count);
        }
        else if (strncmp(line, "usemtl", 6) == 0)
        {
            for (i = 0; i < material_count; ++i)
            {
                if (strcmp(materials[i].name, line + 7) == 0)
                {
                    printf("Setting material for %s to %s\n", mesh->name, materials[i].name);
                    mesh->mat = malloc(sizeof(raw_material_t));
                    raw_material_init(mesh->mat);
                    raw_material_copy(mesh->mat, &materials[i]);
                    break;
                }
            }
        }

        if (all_verts_index >= all_verts_cap - 1)
        {
            all_verts_cap *= 2;
            all_verts = realloc(all_verts, sizeof(float) * all_verts_cap * 3);
            CHECK_MEM(all_verts);
        }

        if (all_norms_index >= all_norms_cap - 1)
        {
            all_norms_cap *= 2;
            all_norms = realloc(all_norms, sizeof(float) * all_norms_cap * 3);
            CHECK_MEM(all_norms);
        }

        if (all_txcds_index >= all_txcds_cap - 1)
        {
            all_txcds_cap *= 2;
            all_txcds = realloc(all_txcds, sizeof(float) * all_txcds_cap * 2);
            CHECK_MEM(all_txcds);
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
            vec3f_copy(&mesh->norms[j * 3 + 1], &mesh->norms[j * 3]);
            vec3f_copy(&mesh->norms[j * 3 + 2], &mesh->norms[j * 3]);
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

    LOG_INFO("Loaded %s: Verts %d, Norms %d, Tex Coords %d", filename, verts_loaded, norms_loaded, txcds_loaded);

    return true;

error:

    free(all_verts);
    free(all_norms);
    free(all_txcds);

    fclose(fp);
    free(line);

    return false;
}
