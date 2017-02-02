#ifndef VIEW3D_CAMERA_H
#define VIEW3D_CAMERA_H

#include "view3d.h"
#include <stdbool.h>

typedef enum camera_dir {
  CAM_DIR_UP,
  CAM_DIR_DOWN,
  CAM_DIR_LEFT,
  CAM_DIR_RIGHT,
  CAM_DIR_FORWARD,
  CAM_DIR_BACK
} camera_dir_t;

typedef struct camera
{
  mat4x4_t proj;
  mat4x4_t view;

  vec3f_t _pos;
  vec3f_t _dir;
  vec3f_t _up;
  vec3f_t _look_at;
  vec3f_t _pos_delta;

  float _pitch;
  float _yaw;

  float _aspect;
  float _vnear;
  float _vfar;
  float _fov;

  bool _invalid_proj;
  bool _invalid_view;
} camera_t;

void camera_init(camera_t *this);
void camera_print(camera_t *this);

void camera_set_aspect(camera_t *this, float width, float height);
void camera_set_clip(camera_t *this, float vnear, float vfar);
void camera_set_fov(camera_t *this, float fov);

void camera_set_pos(camera_t *this, vec3f_t pos);
void camera_set_dir(camera_t *this, vec3f_t dir);
void camera_set_look_at(camera_t *this, vec3f_t look_at);
void camera_set_up(camera_t *this, vec3f_t up);

void camera_move(camera_t *this, camera_dir_t dir, float amount);

void camera_change_pitch(camera_t *this, float angle);
void camera_change_yaw(camera_t *this, float angle);

void camera_update(camera_t *this);

#endif // VIEW3D_CAMERA_H
