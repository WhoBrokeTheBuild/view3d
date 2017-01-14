#ifndef VIEW3D_CAMERA_H
#define VIEW3D_CAMERA_H

#include <view3d.h>
#include <stdbool.h>

typedef struct
{
    mat4x4_t proj;
    mat4x4_t view;

    bool _invalid_proj;
    bool _invalid_view;

    float _aspect;
    float _vnear;
    float _vfar;
    float _fov;

    quat_t _orient;

} camera_t;

void camera_init(camera_t* this, float width, float height, float vnear, float vfar, float fov);
void camera_print(camera_t* this);

void camera_look_at(camera_t* this, const vec3f_t eye, const vec3f_t center, const vec3f_t up);

// TODO
void camera_rotate(camera_t* this, float angle, const vec3f_t axis);
void camera_translate(camera_t* this, const vec3f_t vec);
void camera_scale(camera_t* this, const vec3f_t scale);

void camera_update(camera_t* this);

void camera_set_aspect(camera_t* this, float width, float height);
void camera_set_vnear_vfar(camera_t* this, float vnear, float vfar);
void camera_set_fov(camera_t* this, float fov);

#endif // VIEW3D_CAMERA_H
