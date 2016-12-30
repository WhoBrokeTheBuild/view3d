#include <GL/glew.h>

#include <GL/freeglut.h>
#include <glmm/glmm.h>
#include <stdio.h>

#include <debug.h>
#include <camera.h>
#include <shaders.h>

#define WIDTH 1024
#define HEIGHT 768

GLuint g_program;
GLuint g_vao;

void update()
{
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    LOG_INFO("Rendering");

    glUseProgram(g_program);
    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glutSwapBuffers();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("View3D");

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "%s\n", glewGetErrorString(err));
        goto error;
    }

    int glut_ver = glutGet(GLUT_VERSION);
    int glut_pat = glut_ver % 100;
    int glut_min = ((glut_ver - glut_pat) % 10000) / 100;
    int glut_maj = glut_ver / 10000;

    printf("Running OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("Running (Free)GLUT Version: %d.%d.%d\n", glut_maj, glut_min, glut_pat);
    printf("Running GLEW Version: %d.%d.%d\n", GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);

    glutDisplayFunc(&render);

    /*
    camera_t cam;
    vec3f_t eye, center, up;
    camera_init(&cam, WIDTH, HEIGHT, 0.1f, 1000.0f, 45.0f);
    vec3f_init(eye, 0.0f);
    vec3f_init(center, 1.0f);
    vec3f_init(up, 0.0f);
    up[1] = 1.0f;
    camera_look_at(&cam, eye, center, up);
    camera_print(&cam);
    */

    float points[] = {
         0.0f,  0.5f,  0.0f,
         0.5f, -0.5f,  0.0f,
        -0.5f, -0.5f,  0.0f
    };

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

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

    glutMainLoop();

    return 0;

error:

    return 1;
}