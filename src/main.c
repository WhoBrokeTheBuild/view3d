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

bool g_rotating = true;

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

    if (g_rotating)
    {
        vec3f_t axis = { 0.0f, 1.0f, 0.0f };
        mat4x4_rotate(g_model_mat, GLMM_RAD(1.0f), axis);
    }

    mat4x4_copy(g_render_data.model, g_model_mat);
    mat4x4_xmul(g_render_data.mvp, g_cam.proj, g_cam.view);
    mat4x4_mul(g_render_data.mvp, g_model_mat);

    model_draw(&g_model);

    glutSwapBuffers();
}

void resize_cb(GLint width, GLint height)
{
    camera_set_aspect(&g_cam, width, height);
}

void special_key_cb(int key, int x, int y)
{
    int mods;
    float speed = 0.5f;
    float turn = GLMM_RAD(10.0f);

    mods = glutGetModifiers();

    switch (key)
    {
    case GLUT_KEY_UP:

        if ((mods & GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_SHIFT)
        {
            camera_change_pitch(&g_cam, turn);
        }
        else
        {
            camera_move(&g_cam, CAM_DIR_FORWARD, speed);
        }

        break;
    case GLUT_KEY_DOWN:

        if ((mods & GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_SHIFT)
        {
            camera_change_pitch(&g_cam, -turn);
        }
        else
        {
            camera_move(&g_cam, CAM_DIR_BACK, speed);
        }

        break;
    case GLUT_KEY_LEFT:

        if ((mods & GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_SHIFT)
        {
            camera_change_yaw(&g_cam, turn);
        }
        else
        {
            camera_move(&g_cam, CAM_DIR_LEFT, speed);
        }

        break;
    case GLUT_KEY_RIGHT:

        if ((mods & GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_SHIFT)
        {
            camera_change_yaw(&g_cam, -turn);
        }
        else
        {
            camera_move(&g_cam, CAM_DIR_RIGHT, speed);
        }

        break;
    case GLUT_KEY_PAGE_UP:

        camera_move(&g_cam, CAM_DIR_UP, speed);

        break;
    case GLUT_KEY_PAGE_DOWN:

        camera_move(&g_cam, CAM_DIR_DOWN, speed);

        break;
    }

    camera_update(&g_cam);
}

void key_cb(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1':
    {
        GLint old_mode[2];
        glGetIntegerv(GL_POLYGON_MODE, old_mode);
        if (old_mode[1] == GL_LINE)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
    }
    break;
    case '2':
    {
        g_rotating = !g_rotating;
    }
    break;
    case '-':
    case '_':
    {
        vec3f_t scale_down = { 0.5f, 0.5f, 0.5f };
        mat4x4_scale(g_model_mat, scale_down);
    }
    break;
    case '+':
    case '=':
    {
        vec3f_t scale_up = { 2.0f, 2.0f, 2.0f };
        mat4x4_scale(g_model_mat, scale_up);
    }
    break;
    }
}

void print_versions()
{
    int glut_ver = glutGet(GLUT_VERSION);
    int glut_pat = glut_ver % 100;
    int glut_min = ((glut_ver - glut_pat) % 10000) / 100;
    int glut_maj = glut_ver / 10000;

    LOG_INFO("GLMM Version: %s", GLMM_VER_STRING);
    LOG_INFO("OpenGL Version: %s", glGetString(GL_VERSION));
    LOG_INFO("(Free)GLUT Version: %d.%d.%d", glut_maj, glut_min, glut_pat);
    LOG_INFO("GLEW Version: %d.%d.%d", GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutDisplayFunc(&render_cb);
    glutIdleFunc(&render_cb);
    glutReshapeFunc(&resize_cb);
    glutSpecialFunc(&special_key_cb);
    glutKeyboardFunc(&key_cb);

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


    vec3f_t up = { 0.0f, 1.0f, 0.0f };
    vec3f_t eye;
    vec3f_t look_at;

    vec3f_copy(eye, g_model.max);
    vec3f_muls(eye, 2.0f);
    vec3f_xadd(look_at, g_model.min, g_model.max);
    vec3f_muls(look_at, 0.5f);

    camera_init(&g_cam);
    camera_set_aspect(&g_cam, START_WIDTH, START_HEIGHT);
    camera_set_clip(&g_cam, 0.001f, 1000.0f);
    camera_set_fov(&g_cam, GLMM_RAD(45.0f));

    camera_set_pos(&g_cam, eye);
    camera_set_look_at(&g_cam, look_at);
    camera_set_up(&g_cam, up);

    camera_update(&g_cam);
    camera_print(&g_cam);

    vec3f_copy(g_render_data.eye_pos, eye);
    vec3f_copy(g_render_data.light_pos, eye);

    mat4x4_init(g_model_mat, 1.0f);

    glutMainLoop();

    return 0;

error:

    return 1;
}
