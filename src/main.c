#include <GL/glew.h>

#include <GL/freeglut.h>
#include <glmm/glmm.h>
#include <stdio.h>
#include <time.h>

#include <camera.h>
#include <debug.h>
#include <shaders.h>

#define WIDTH 1024
#define HEIGHT 768

GLuint g_program;
GLuint g_vao;

camera_t g_cam;
mat4x4_t g_model;

GLint g_mvp_loc, g_color_loc;

float g_rot = 0.0f;
vec3f_t g_rot_axis = { 0.0f, 0.0f, 1.0f };

vec3f_t g_color;
unsigned int g_color_timer = 0;

void update()
{
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera_update(&g_cam);

    mat4x4_init(g_model, 1.0f);
    mat4x4_rotate(g_model, g_rot, g_rot_axis);

    if (g_color_timer <= 0)
    {
        g_color[0] = (float)(rand() % 255) / 255.0f;
        g_color[1] = (float)(rand() % 255) / 255.0f;
        g_color[2] = (float)(rand() % 255) / 255.0f;
        g_color_timer = 5;
    }
    --g_color_timer;

    g_rot += 0.1f;

    mat4x4_t mvp;
    mat4x4_mul(mvp, g_cam.proj, g_cam.view);
    mat4x4_mul(mvp, mvp, g_model);

    glUseProgram(g_program);
    glUniformMatrix4fv(g_mvp_loc, 1, GL_FALSE, (GLfloat *)mvp);
    glUniform3fv(g_color_loc, 1, (GLfloat *)g_color);
    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, 24);

    glutSwapBuffers();
}

int main(int argc, char **argv)
{
    srand(time(0));

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
    glutIdleFunc(&render);

    camera_init(&g_cam, WIDTH, HEIGHT, 0.1f, 1000.0f, GLMM_RAD(45.0f));

    vec3f_t eye = { 4.0f, 3.0f, 3.0f };
    vec3f_t center = { 0.0f, 0.0f, 0.0f };
    vec3f_t up = { 0.0f, 1.0f, 0.0f };
    camera_look_at(&g_cam, eye, center, up);

    camera_update(&g_cam);
    camera_print(&g_cam);

    float points[] = {
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f,-1.0f, 
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
    };

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
