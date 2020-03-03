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

public:
    float x, y, width, height;

    UI_Panel();
    UI_Panel(float panel_x, float panel_y, float panel_width, float panel_height);

    virtual bool click_handler(float click_x, float click_y);
    virtual void render();
};

class Texture_Panel : public UI_Panel {
    GLuint texture_id;
    void (*click_ptr)(void);

public:
    Texture_Panel();
    Texture_Panel(GLuint texture, float panel_x, float panel_y, float panel_width, float panel_height, void (*on_click_ptr)(void) = nullptr);

    virtual void render();
    virtual bool click_handler(float click_x, float click_y);
};

class BounceAnimation : public Animation {
    float scale, offset, frequency;

public:
    BounceAnimation();
    BounceAnimation(float maximum, float anim_scale, float anim_offset, float anim_frequency);

    virtual float modify_feature(float feature);
};

class ScaleAnimation : public Animation {
    float scale, offset, frequency;

public:
    ScaleAnimation();
    ScaleAnimation(float maximum, float anim_scale, float anim_offset, float anim_frequency);

    virtual float modify_feature(float feature);
};