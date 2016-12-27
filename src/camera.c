#include "camera.h"
#include <math.h>

void camera_init(camera_t *this)
{
    quat_init(this->orient);
}

void camera_set_proj(camera_t* this, float width, float height, float near, float far, float fov)
{
    static float D2R = GLMM_PI / 180.0f;

    float aspect = height / width;
    float yScale = 1.0f / tanf(D2R * fov / 2.0f);
    float xScale = yScale / aspect;
    float nearmfar = near - far;
    mat4x4_t tmp = {
        { xScale, 0.0f, 0.0f, 0.0f },
        { 0.0f, yScale, 0.0f, 0.0f },
        { 0.0f, 0.0f, (far + near) / (near - far), -1.0f },
        { 0.0f, 0.0f, 2.0f * far * near / (near - far) }
    };
    mat4x4_copy(this->proj, tmp);
}

void camera_look_at(camera_t *this, const vec3f_t eye, const vec3f_t up, const vec3f_t lookAt)
{
    vec3f_t right;
    vec3f_cross(right, up, lookAt);

    mat3x3_t m;
    vec3f_copy(m[0], right);
    vec3f_copy(m[1], up);
    vec3f_copy(m[2], lookAt);

    this->orient[3] = sqrtf(1.0f + m[0][0] + m[1][1] + m[2][2]) * 0.5f;

    float recip = 1.0f / (4.0f * this->orient[3]);

    this->orient[0] = ( m[2][1] - m[1][2] ) * recip;
    this->orient[1] = ( m[0][2] - m[2][0] ) * recip;
    this->orient[3] = ( m[1][0] - m[0][1] ) * recip;

    quat_norm(this->orient);
}

void camera_print(camera_t *this)
{
    quat_print(this->orient);
}
