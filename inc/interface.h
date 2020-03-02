#pragma once

#include "stdio.h"

class Animation {
    float timer_value, timer_maximum;

public:
    Animation();
    Animation(float maximum);

    void reset();
    void progress_timer(float dt);
    virtual float modify_feature(float feature);
    float getTimerValue();
    float getTimerMaximum();
};

class BounceAnimation : public Animation {
    float scale, offset, frequency;

public:
    BounceAnimation();
    BounceAnimation(float maximum, float anim_scale, float anim_offset, float anim_frequency);

    float modify_feature(float feature);
};

class ScaleAnimation : public Animation {
    float scale, offset, frequency;

public:
    ScaleAnimation();
    ScaleAnimation(float maximum, float anim_scale, float anim_offset, float anim_frequency);

    float modify_feature(float feature);
};