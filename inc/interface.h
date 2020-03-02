#pragma once

#include <stdio.h>
#include <GL/glew.h>

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

class UI_Panel {
    float x, y, width, height;

public:
    UI_Panel();
    UI_Panel(float panel_x, float panel_y, float panel_width, float panel_height);

    virtual void click_handler(float click_x, float click_y);
    virtual void render();

    float getX();
    float getY();
    float getWidth();
    float getHeight();
};

class Texture_Panel : public UI_Panel {
    GLuint texture_id;

public:
    Texture_Panel();
    Texture_Panel(GLuint texture, float panel_x, float panel_y, float panel_width, float panel_height);

    virtual void render();
}

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