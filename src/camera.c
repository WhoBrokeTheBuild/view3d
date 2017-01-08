#include "camera.h"
#include <math.h>

void camera_init(camera_t *this, float width, float height, float near, float far, float fov)
{
    mat4x4_init(this->proj, 1.0f);
    mat4x4_init(this->view, 1.0f);

    this->_invalid_proj = true;
    this->_invalid_view = true;
    this->_aspect = width / height;
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
    glmm_look_at(this->view, eye, center, up);
}

void camera_update(camera_t *this)
{
    if (this->_invalid_view || this->_invalid_proj)
    {
        //LOG_INFO("Recalculating View Matrix");
        this->_invalid_view = false;
    }

    if (this->_invalid_proj)
    {
        //LOG_INFO("Recalculating Projection Matrix");
        glmm_perspective(this->proj, this->_aspect, this->_near, this->_far, this->_fov);
        this->_invalid_proj = false;
    }
}

void camera_rotate(camera_t* this, float angle, const vec3f_t axis)
{
    glmm_mat4x4_rotate(this->view, angle, axis);
    this->_invalid_view = true;
}

void camera_translate(camera_t* this, const vec3f_t vec)
{
    glmm_mat4x4_translate(this->view, vec);
    this->_invalid_view = true;
}

void camera_scale(camera_t* this, const vec3f_t scale)
{
    glmm_mat4x4_scale(this->view, scale);
    this->_invalid_view = true;
}

void camera_set_aspect(camera_t *this, float width, float height)
{
    this->_aspect = width / height;
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
