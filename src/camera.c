#include "camera.h"
#include <math.h>

void camera_init(camera_t *this)
{
    mat4x4_init(this->proj, 1.0f);
    mat4x4_init(this->view, 1.0f);

    vec3f_init(this->_pos, 0.0f);
    vec3f_init(this->_dir, 0.0f);
    vec3f_init(this->_up, 0.0f);
    this->_up[1] = 1.0f;

    this->_pitch = 0.0f;
    this->_yaw = 0.0f;

    this->_aspect = 0.0f;
    this->_vnear = 1000.0f;
    this->_vfar = 0.1f;
    this->_fov = 45.0f;

    this->_invalid_proj = true;
    this->_invalid_view = true;
}

void camera_print(camera_t *this)
{
    printf("Proj:\n");
    mat4x4_print(this->proj);
    printf("\n");

    printf("View:\n");
    mat4x4_print(this->view);
    printf("\n");

    printf("Pos:\n");
    vec3f_print(this->_pos);
    printf("\n");

    printf("Dir:\n");
    vec3f_print(this->_dir);
    printf("\n");

    printf("Up:\n");
    vec3f_print(this->_up);
    printf("\n");

    printf("Look At:\n");
    vec3f_print(this->_look_at);
    printf("\n");
}

void camera_set_aspect(camera_t *this, float width, float height)
{
    this->_aspect = width / height;
    this->_invalid_proj = true;
}

void camera_set_clip(camera_t *this, float vnear, float vfar)
{
    this->_vnear = vnear;
    this->_vfar = vfar;
    this->_invalid_proj = true;
}

void camera_set_fov(camera_t *this, float fov)
{
    this->_fov = fov;
    this->_invalid_proj = true;
}

void camera_set_pos(camera_t *this, vec3f_t pos)
{
    vec3f_copy(this->_pos, pos);
    this->_invalid_view = true;
}

void camera_set_dir(camera_t *this, vec3f_t dir)
{
    vec3f_copy(this->_dir, dir);
    this->_invalid_view = true;
}

void camera_set_look_at(camera_t* this, vec3f_t look_at)
{
    vec3f_copy(this->_look_at, look_at);
    this->_invalid_view = true;
}

void camera_set_up(camera_t *this, vec3f_t up)
{
    vec3f_copy(this->_up, up);
    this->_invalid_view = true;
}

void camera_move(camera_t *this, camera_dir_t dir, float amount)
{
    vec3f_t tmp, left;

    switch (dir)
    {
    case CAM_DIR_UP:

        vec3f_mul_scalar(tmp, this->_up, amount);
        vec3f_add(this->_pos_delta, this->_pos_delta, tmp);

        break;
    case CAM_DIR_DOWN:

        vec3f_mul_scalar(tmp, this->_up, amount);
        vec3f_sub(this->_pos_delta, this->_pos_delta, tmp);

        break;
    case CAM_DIR_LEFT:

        vec3f_cross(left, this->_dir, this->_up);
        vec3f_mul_scalar(left, left, amount);
        vec3f_sub(this->_pos_delta, this->_pos_delta, left);

        break;
    case CAM_DIR_RIGHT:

        vec3f_cross(left, this->_dir, this->_up);
        vec3f_mul_scalar(left, left, amount);
        vec3f_add(this->_pos_delta, this->_pos_delta, left);

        break;
    case CAM_DIR_FORWARD:

        vec3f_mul_scalar(tmp, this->_dir, amount);
        vec3f_add(this->_pos_delta, this->_pos_delta, tmp);

        break;
    case CAM_DIR_BACK:

        vec3f_mul_scalar(tmp, this->_dir, amount);
        vec3f_sub(this->_pos_delta, this->_pos_delta, tmp);

        break;
    }

    this->_invalid_view = true;
}

void camera_change_pitch(camera_t* this, float angle)
{
    this->_pitch += angle;

    if (this->_pitch > GLMM_2PI)
    {
        this->_pitch -= GLMM_2PI;
    }

    this->_invalid_view = true;
}

void camera_change_yaw(camera_t* this, float angle)
{
    this->_yaw += angle;

    if ((this->_pitch > GLMM_PI * 0.5f && this->_pitch < GLMM_PI * 1.5f) || (this->_pitch < -GLMM_PI * 0.5f && this->_pitch > -GLMM_PI * 1.5f))
    {
        this->_yaw -= angle;
    } else
    {
        this->_yaw += angle;
    }

    if (this->_yaw > GLMM_2PI)
    {
        this->_yaw -= GLMM_2PI;
    }

    this->_invalid_view = true;
}

void camera_update(camera_t *this)
{
    if (this->_invalid_view)
    {
        vec3f_t pitch_axis;
        quat_t pitch_quat, yaw_quat, tmp;

        vec3f_sub(this->_dir, this->_look_at, this->_pos);
        vec3f_norm(this->_dir);

        vec3f_cross(pitch_axis, this->_dir, this->_up);
        quat_angle_axis(pitch_quat, this->_pitch, pitch_axis);
        quat_angle_axis(yaw_quat, this->_yaw, this->_up);

        quat_cross(tmp, pitch_quat, yaw_quat);
        quat_norm(tmp);

        vec3f_rotate_by_quat(this->_dir, this->_dir, tmp);

        vec3f_add(this->_pos, this->_pos, this->_pos_delta);
        vec3f_add(this->_look_at, this->_pos, this->_dir);

        glmm_look_at(this->view, this->_pos, this->_look_at, this->_up);

        this->_yaw *= 0.5f;
        this->_pitch *= 0.5f;
        vec3f_init(this->_pos_delta, 0.0f);

        this->_invalid_view = false;
    }

    if (this->_invalid_proj)
    {
        glmm_perspective(this->proj, this->_aspect, this->_vnear, this->_vfar, this->_fov);
        this->_invalid_proj = false;
    }
}
