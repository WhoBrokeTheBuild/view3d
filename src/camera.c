#include "camera.h"
#include <math.h>

void camera_init(camera_t *this)
{
    mat4x4_init(this->view);
}

void camera_set_proj(camera_t* this, float width, float height, float near, float far, float fov)
{
    static float D2R = GLMM_PI / 180.0f;

    float aspect = height / width;
    float yScale = 1.0f / tanf(D2R * fov / 2.0f);
    float xScale = yScale / aspect;
    mat4x4_t tmp = {
        { xScale, 0.0f, 0.0f, 0.0f },
        { 0.0f, yScale, 0.0f, 0.0f },
        { 0.0f, 0.0f, (far + near) / (near - far), -1.0f },
        { 0.0f, 0.0f, 2.0f * far * near / (near - far) }
    };
    mat4x4_copy(this->proj, tmp);
}

void camera_look_at(camera_t *this, const vec3f_t eye, const vec3f_t center, const vec3f_t up)
{
    look_at(this->view, eye, center, up);
}

void camera_print(camera_t *this)
{
    printf("View:\n");
    mat4x4_print(this->view);
    printf("\n");

    //quat_print(this->orient);
}
