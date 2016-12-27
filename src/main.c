#include <glmm/glmm.h>
#include <stdio.h>

#include "camera.h"

int main(int argc, char **argv)
{
    camera_t cam;
    camera_init(&cam);

    vec3f_t eye, dir, lookAt;
    vec3f_init(eye);
    vec3f_init(dir);
    vec3f_init(lookAt);

    dir[2] = 1.0f; 

    vec3f_set(lookAt, 1.0f);

    camera_look_at(&cam, eye, dir, lookAt);

    camera_print(&cam);

    return 0;
}
