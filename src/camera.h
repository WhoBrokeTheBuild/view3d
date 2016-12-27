#ifndef VIEW3D_CAMERA_H
#define VIEW3D_CAMERA_H

#include <glmm/glmm.h>

typedef struct 
{
    mat3x3_t proj;
    mat3x3_t view;
    quat_t orient; 
} camera_t;

void camera_init(camera_t* this);
void camera_set_proj(camera_t* this, float width, float height, float near, float far, float fov);
void camera_look_at(camera_t* this, const vec3f_t eye, const vec3f_t up, const vec3f_t x);

void camera_print(camera_t* this);

#endif // VIEW3D_CAMERA_H
