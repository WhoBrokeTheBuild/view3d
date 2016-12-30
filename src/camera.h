#ifndef VIEW3D_CAMERA_H
#define VIEW3D_CAMERA_H

#include <glmm/glmm.h>
#include <stdbool.h>

typedef struct 
{
    mat4x4_t proj;
    mat4x4_t view;

    bool _invalid_proj;
    bool _invalid_view;

    float _aspect;
    float _near;
    float _far;
    float _fov;

    quat_t _orient;
    
} camera_t;

void camera_init(camera_t* this, float width, float height, float near, float far, float fov);
void camera_print(camera_t* this);

void camera_look_at(camera_t* this, const vec3f_t eye, const vec3f_t center, const vec3f_t up);

// TODO
void camera_rotate();
void camera_translate();
void camera_scale();

void camera_update(camera_t* this);

void camera_set_aspect(camera_t* this, float width, float height);
void camera_set_near_far(camera_t* this, float near, float far);
void camera_set_fov(camera_t* this, float fov);

#endif // VIEW3D_CAMERA_H
