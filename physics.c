#include "physics.h"

bool ray_voxel_colliding(vec3 point, vec3 direction, vec3 box_min, vec3 box_max, int *contact_normal)
{
    float oodx = 1.0 / direction[0];
    float oody = 1.0 / direction[1];
    float oodz = 1.0 / direction[2];

    float t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0;
    if (fabs(direction[0]) > DBL_EPSILON)
    {
        t1 = (box_min[0] - point[0]) * oodx;
        t2 = (box_max[0] - point[0]) * oodx;
    }
    if (fabs(direction[1]) > DBL_EPSILON)
    {
        t3 = (box_min[1] - point[1]) * oody;
        t4 = (box_max[1] - point[1]) * oody;
    }
    if (fabs(direction[2]) > DBL_EPSILON)
    {
        t5 = (box_min[2] - point[2]) * oodz;
        t6 = (box_max[2] - point[2]) * oodz;
    }

    float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
    float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));
    if (tmin > tmax || tmax < 0.0)
        return false;

    if (tmin == t1)
    {
        // x
        *contact_normal = 0;
    }
    if (tmin == t2)
    {
        // x
        *contact_normal = 1;
    }
    if (tmin == t3)
    {
        // y
        *contact_normal = 2;
    }
    if (tmin == t4)
    {
        // y
        *contact_normal = 3;
    }
    if (tmin == t5)
    {
        // z
        *contact_normal = 4;
    }
    if (tmin == t6)
    {
        // z
        *contact_normal = 5;
    }

    return true;
}