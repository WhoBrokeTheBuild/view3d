#include <glmm/glmm.h>
#include <stdio.h>

#include "camera.h"

int main(int argc, char **argv)
{
    camera_t cam;
    vec3f_t eye, center, up;

    camera_init(&cam);
    
    vec3f_init(eye, 0.0f);
    vec3f_init(center, 1.0f);
    vec3f_init(up, 0.0f);
    up[1] = 1.0f; 

    camera_look_at(&cam, eye, center, up);

    camera_print(&cam);

    return 0;
}
