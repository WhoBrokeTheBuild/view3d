#include "camera.h"
#include <math.h>

void camera_init(camera_t *this, float width, float height, float near, float far, float fov)
{
    mat4x4_init(this->proj);
    mat4x4_init(this->view);

    this->_invalid_proj = true;
    this->_invalid_view = true;
    this->_aspect = height / width;
    this->_near = near;
    this->_far = far;
    this->_fov = fov;
    quat_init(this->_orient);
}

void camera_print(camera_t *this)
{
    printf("Proj:\n");
    mat4x4_print(this->proj);
    printf("\n");

    printf("View:\n");
    mat4x4_print(this->view);
    printf("\n");

    printf("Orient:\n");
    quat_print(this->_orient);
    printf("\n");
}

void camera_look_at(camera_t *this, const vec3f_t eye, const vec3f_t center, const vec3f_t up)
{
    look_at(this->view, eye, center, up);
}

void camera_update(camera_t *this)
{
    static float D2R = GLMM_PI / 180.0f;

    if (this->_invalid_proj)
    {
        float y_scale = 1.0f / tanf(D2R * this->_fov / 2.0f);
        float x_scale = y_scale / this->_aspect;
        mat4x4_t tmp = {
            { x_scale, 0.0f, 0.0f, 0.0f },
            { 0.0f, y_scale, 0.0f, 0.0f },
            { 0.0f, 0.0f, (this->_far + this->_near) / (this->_near - this->_far), -1.0f },
            { 0.0f, 0.0f, 2.0f * this->_far * this->_near / (this->_near - this->_far) }
        };
        mat4x4_copy(this->proj, tmp);
    }

    if (this->_invalid_view)
    {
    }
}

void camera_set_aspect(camera_t *this, float width, float height)
{
    this->_aspect = height / width;
    this->_invalid_proj = true;
}

void camera_set_fov(camera_t *this, float fov)
{
    this->_fov = fov;
    this->_invalid_proj = true;
}

void camera_set_near_far(camera_t *this, float near, float far)
{
    this->_near = near;
    this->_far = far;
    this->_invalid_proj = true;
}
