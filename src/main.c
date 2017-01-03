#include <GL/glew.h>

#include <GL/freeglut.h>
#include <glmm/glmm.h>
#include <stdio.h>
#include <time.h>

#include <camera.h>
#include <debug.h>
#include <shaders.h>

GLuint g_program;
GLuint g_vao;

camera_t g_cam;
mat4x4_t g_model;

GLint g_mvp_loc, g_color_loc;

float g_rot = 10.0f;
int g_rot_dir = 0;
vec3f_t g_rot_axis[] = {
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f }
};
int g_rot_change_timeout = 0;

void update()
{
}

void render_cb()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera_update(&g_cam);

    if (g_rot_change_timeout <= 0)
    {
        g_rot_dir = rand() % 6;
        g_rot_change_timeout = 100;
    }
    --g_rot_change_timeout;

    mat4x4_rotate(g_model, GLMM_RAD(1.0f), g_rot_axis[g_rot_dir]);

    mat4x4_t mvp;
    mat4x4_mul(mvp, g_cam.proj, g_cam.view);
    mat4x4_mul(mvp, mvp, g_model);

    glUseProgram(g_program);
    glUniformMatrix4fv(g_mvp_loc, 1, GL_FALSE, (GLfloat *)mvp);
    glBindVertexArray(g_vao);
    glDrawArrays(GL_LINES, 0, 114);

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
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glutDisplayFunc(&render_cb);
    glutIdleFunc(&render_cb);
    glutReshapeFunc(&resize_cb);

    return true;

error:

    return false;
}

int main(int argc, char **argv)
{
    srand(time(0));

    const int START_WIDTH = 1024;
    const int START_HEIGHT = 768;

    float points[] = {
        0.38268343, 0.92387953, 0.0000000e+0,
        0.35355339, 0.92387953, 0.14644661,
        0.27059805, 0.92387953, 0.27059805,
        0.14644661, 0.92387953, 0.35355339,
        2.3432602e-17, 0.92387953, 0.38268343,
        -0.14644661, 0.92387953, 0.35355339,
        -0.27059805, 0.92387953, 0.27059805,
        -0.35355339, 0.92387953, 0.14644661,
        -0.38268343, 0.92387953, 4.6865204e-17,
        -0.35355339, 0.92387953, -0.14644661,
        -0.27059805, 0.92387953, -0.27059805,
        -0.14644661, 0.92387953, -0.35355339,
        -7.0297806e-17, 0.92387953, -0.38268343,
        0.14644661, 0.92387953, -0.35355339,
        0.27059805, 0.92387953, -0.27059805,
        0.35355339, 0.92387953, -0.14644661,
        0.70710678, 0.70710678, 0.0000000e+0,
        0.65328148, 0.70710678, 0.27059805,
        0.50000000, 0.70710678, 0.50000000,
        0.27059805, 0.70710678, 0.65328148,
        4.3297803e-17, 0.70710678, 0.70710678,
        -0.27059805, 0.70710678, 0.65328148,
        -0.50000000, 0.70710678, 0.50000000,
        -0.65328148, 0.70710678, 0.27059805,
        -0.70710678, 0.70710678, 8.6595606e-17,
        -0.65328148, 0.70710678, -0.27059805,
        -0.50000000, 0.70710678, -0.50000000,
        -0.27059805, 0.70710678, -0.65328148,
        -1.2989341e-16, 0.70710678, -0.70710678,
        0.27059805, 0.70710678, -0.65328148,
        0.50000000, 0.70710678, -0.50000000,
        0.65328148, 0.70710678, -0.27059805,
        0.92387953, 0.38268343, 0.0000000e+0,
        0.85355339, 0.38268343, 0.35355339,
        0.65328148, 0.38268343, 0.65328148,
        0.35355339, 0.38268343, 0.85355339,
        5.6571306e-17, 0.38268343, 0.92387953,
        -0.35355339, 0.38268343, 0.85355339,
        -0.65328148, 0.38268343, 0.65328148,
        -0.85355339, 0.38268343, 0.35355339,
        -0.92387953, 0.38268343, 1.1314261e-16,
        -0.85355339, 0.38268343, -0.35355339,
        -0.65328148, 0.38268343, -0.65328148,
        -0.35355339, 0.38268343, -0.85355339,
        -1.6971392e-16, 0.38268343, -0.92387953,
        0.35355339, 0.38268343, -0.85355339,
        0.65328148, 0.38268343, -0.65328148,
        0.85355339, 0.38268343, -0.35355339,
        1.00000000, 6.1232340e-17, 0.0000000e+0,
        0.92387953, 6.1232340e-17, 0.38268343,
        0.70710678, 6.1232340e-17, 0.70710678,
        0.38268343, 6.1232340e-17, 0.92387953,
        6.1232340e-17, 6.1232340e-17, 1.00000000,
        -0.38268343, 6.1232340e-17, 0.92387953,
        -0.70710678, 6.1232340e-17, 0.70710678,
        -0.92387953, 6.1232340e-17, 0.38268343,
        -1.00000000, 6.1232340e-17, 1.2246468e-16,
        -0.92387953, 6.1232340e-17, -0.38268343,
        -0.70710678, 6.1232340e-17, -0.70710678,
        -0.38268343, 6.1232340e-17, -0.92387953,
        -1.8369702e-16, 6.1232340e-17, -1.00000000,
        0.38268343, 6.1232340e-17, -0.92387953,
        0.70710678, 6.1232340e-17, -0.70710678,
        0.92387953, 6.1232340e-17, -0.38268343,
        0.92387953, -0.38268343, 0.0000000e+0,
        0.85355339, -0.38268343, 0.35355339,
        0.65328148, -0.38268343, 0.65328148,
        0.35355339, -0.38268343, 0.85355339,
        5.6571306e-17, -0.38268343, 0.92387953,
        -0.35355339, -0.38268343, 0.85355339,
        -0.65328148, -0.38268343, 0.65328148,
        -0.85355339, -0.38268343, 0.35355339,
        -0.92387953, -0.38268343, 1.1314261e-16,
        -0.85355339, -0.38268343, -0.35355339,
        -0.65328148, -0.38268343, -0.65328148,
        -0.35355339, -0.38268343, -0.85355339,
        -1.6971392e-16, -0.38268343, -0.92387953,
        0.35355339, -0.38268343, -0.85355339,
        0.65328148, -0.38268343, -0.65328148,
        0.85355339, -0.38268343, -0.35355339,
        0.70710678, -0.70710678, 0.0000000e+0,
        0.65328148, -0.70710678, 0.27059805,
        0.50000000, -0.70710678, 0.50000000,
        0.27059805, -0.70710678, 0.65328148,
        4.3297803e-17, -0.70710678, 0.70710678,
        -0.27059805, -0.70710678, 0.65328148,
        -0.50000000, -0.70710678, 0.50000000,
        -0.65328148, -0.70710678, 0.27059805,
        -0.70710678, -0.70710678, 8.6595606e-17,
        -0.65328148, -0.70710678, -0.27059805,
        -0.50000000, -0.70710678, -0.50000000,
        -0.27059805, -0.70710678, -0.65328148,
        -1.2989341e-16, -0.70710678, -0.70710678,
        0.27059805, -0.70710678, -0.65328148,
        0.50000000, -0.70710678, -0.50000000,
        0.65328148, -0.70710678, -0.27059805,
        0.38268343, -0.92387953, 0.0000000e+0,
        0.35355339, -0.92387953, 0.14644661,
        0.27059805, -0.92387953, 0.27059805,
        0.14644661, -0.92387953, 0.35355339,
        2.3432602e-17, -0.92387953, 0.38268343,
        -0.14644661, -0.92387953, 0.35355339,
        -0.27059805, -0.92387953, 0.27059805,
        -0.35355339, -0.92387953, 0.14644661,
        -0.38268343, -0.92387953, 4.6865204e-17,
        -0.35355339, -0.92387953, -0.14644661,
        -0.27059805, -0.92387953, -0.27059805,
        -0.14644661, -0.92387953, -0.35355339,
        -7.0297806e-17, -0.92387953, -0.38268343,
        0.14644661, -0.92387953, -0.35355339,
        0.27059805, -0.92387953, -0.27059805,
        0.35355339, -0.92387953, -0.14644661,
        0.0000000e+0, 1.00000000, 0.0000000e+0,
        0.0000000e+0, -1.00000000, 0.0000000e+0
    };

    if (!window_init(argc, argv, START_WIDTH, START_HEIGHT))
    {
        LOG_ERR("Window initialization failed");
        goto error;
    }

    camera_init(&g_cam, START_WIDTH, START_HEIGHT, 0.01f, 10.0f, GLMM_RAD(45.0f));

    vec3f_t eye = { 4.0f, 3.0f, 3.0f };
    vec3f_t center = { 0.0f, 0.0f, 0.0f };
    vec3f_t up = { 0.0f, 1.0f, 0.0f };
    camera_look_at(&g_cam, eye, center, up);

    camera_update(&g_cam);
    camera_print(&g_cam);

    mat4x4_init(g_model, 1.0f);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    shader_info_t shaders[] = {
        { GL_VERTEX_SHADER, "shaders/test.vs.glsl" },
        { GL_FRAGMENT_SHADER, "shaders/test.fs.glsl" },
        { 0, NULL }
    };

    g_program = shader_program_load(shaders);

    g_mvp_loc = glGetUniformLocation(g_program, "mat_mvp");
    g_color_loc = glGetUniformLocation(g_program, "f_color");

    glutMainLoop();

    return 0;

error:

    return 1;
}
