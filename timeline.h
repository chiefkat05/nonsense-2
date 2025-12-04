#ifndef TIMELINE_H
#define TIMELINE_H

typedef struct
{
    unsigned int texture_offset;
    int width_shift, height_shift;
    int x_move_count;
    double timer, timer_speed;
} animation;

void animation_update(animation *anim)
{
    if (anim->timer < 0.0)
    {
        anim->timer = 100.0;
    }

    

    anim->timer -= anim->timer_speed;
}

typedef struct
{

} timeline;

#endif