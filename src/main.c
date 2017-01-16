#include <stdio.h>
#include <time.h>
#include <view3d.h>

#include <camera.h>
#include <model.h>
#include <shaders.h>

typedef struct
{
    mat4x4_t mvp;
    mat4x4_t model;
    vec3f_t eye_pos;
    vec3f_t light_pos;
    GLuint mvp_loc;
    GLuint model_loc;
    GLuint eye_pos_loc;
    GLuint light_pos_loc;
} render_data_t;

camera_t g_cam;
model_t g_model;
render_data_t g_render_data;
mat4x4_t g_model_mat;

void render_data_init_cb(void *in_data, GLuint shader)
{
    render_data_t *data = (render_data_t *)in_data;
    data->mvp_loc = glGetUniformLocation(shader, "u_mvp");
    data->model_loc = glGetUniformLocation(shader, "u_model");
    data->eye_pos_loc = glGetUniformLocation(shader, "u_eye_pos");
    data->light_pos_loc = glGetUniformLocation(shader, "u_light_pos");
}

void render_data_bind_cb(void *in_data)
{
    render_data_t *data = (render_data_t *)in_data;
    glUniformMatrix4fv(data->mvp_loc, 1, GL_FALSE, (GLfloat *)data->mvp);
    glUniformMatrix4fv(data->model_loc, 1, GL_FALSE, (GLfloat *)data->model);
    glUniform3fv(data->eye_pos_loc, 1, (GLfloat *)data->eye_pos);
    glUniform3fv(data->light_pos_loc, 1, (GLfloat *)data->light_pos);
}

void update()
{
}

void render_cb()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera_update(&g_cam);

    vec3f_t axis = { 0.0f, 1.0f, 0.0f };
    mat4x4_rotate(g_model_mat, GLMM_RAD(0.5f), axis);

    mat4x4_copy(g_render_data.model, g_model_mat);
    mat4x4_mul(g_render_data.mvp, g_cam.proj, g_cam.view);
    mat4x4_mul(g_render_data.mvp, g_render_data.mvp, g_model_mat);

    model_draw(&g_model);

    glutSwapBuffers();
}

void resize_cb(GLint width, GLint height)
{
    camera_set_aspect(&g_cam, width, height);
}

void print_versions()
{
    int glut_ver = glutGet(GLUT_VERSION);
    int glut_pat = glut_ver % 100;
    int glut_min = ((glut_ver - glut_pat) % 10000) / 100;
    int glut_maj = glut_ver / 10000;

    printf("Running GLMM Version: %s\n", GLMM_VER_STRING);
    printf("Running OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("Running (Free)GLUT Version: %d.%d.%d\n", glut_maj, glut_min, glut_pat);
    printf("Running GLEW Version: %d.%d.%d\n", GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);
}

bool window_init(int argc, char **argv, int width, int height)
{
    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutCreateWindow("View3D");

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        LOG_ERR("%s\n", glewGetErrorString(err));
        goto error;
    }

    print_versions();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glutDisplayFunc(&render_cb);
    glutIdleFunc(&render_cb);
    glutReshapeFunc(&resize_cb);

    return true;

error:

    return false;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("USAGE v3d [FILENAME]\n");
        return 0;
    }

    srand(time(0));

    const int START_WIDTH = 1024;
    const int START_HEIGHT = 768;

    if (!window_init(argc, argv, START_WIDTH, START_HEIGHT))
    {
        LOG_ERR("Window initialization failed");
        goto error;
    }

    camera_init(&g_cam, START_WIDTH, START_HEIGHT, 1.0f, 1000.0f, GLMM_RAD(45.0f));

    vec3f_t eye = { 150.0f, 150.0f, 150.0f };
    vec3f_t center = { 0.0f, 100.0f, 0.0f };
    vec3f_t up = { 0.0f, 1.0f, 0.0f };
    camera_look_at(&g_cam, eye, center, up);

    camera_update(&g_cam);

    vec3f_copy(g_render_data.eye_pos, eye);

    mat4x4_init(g_model_mat, 1.0f);

    shader_info_t shaders[] = {
        { GL_VERTEX_SHADER, "shaders/default.vs.glsl" },
        { GL_FRAGMENT_SHADER, "shaders/default.fs.glsl" },
        { 0, NULL }
    };

    GLuint def_shader = shader_program_load(shaders);

    shader_data_t data = {
        (void *)&g_render_data,
        &render_data_init_cb,
        &render_data_bind_cb
    };

    bool ret = model_load_from_file(&g_model, argv[1], NULL, def_shader, &data);
    CHECK(ret, "Failed to load model");

    glutMainLoop();

    return 0;

error:

    return 1;
}
