#pragma once

#include <stdio.h>
#include <GL/glew.h>

class Timer {
    float timer_value, timer_maximum;
    bool one_shot;
    void (*complete_ptr)(void);

public:
    Timer();
    Timer(float maximum, bool repeat = true, void (*on_complete_ptr)(void) = nullptr);

    void reset();
    void progress_timer(float dt);
    float getTimerValue();
    float getTimerMaximum();
};

class Animation {
    Timer *timer_ptr;

public:
    Animation();
    Animation(Timer *timer);
    
    float getTimerValue();
    float getTimerMaximum();
    virtual float modify_feature(float feature);
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
    BounceAnimation(Timer *timer, float anim_scale, float anim_offset, float anim_frequency);

    virtual float modify_feature(float feature);
};

class ScaleAnimation : public Animation {
    float scale, offset, frequency;

public:
    ScaleAnimation();
    ScaleAnimation(Timer *timer, float anim_scale, float anim_offset, float anim_frequency);

    virtual float modify_feature(float feature);
};

class CubicBezierAnimation : public Animation {
    float x0, x1, y0, y1;

    float get_bezier_term(int i, float t);
    float get_bezier_x_gradient(float t);

    float get_t_given_x(float x);
    float get_y_given_t(float t);
    float get_x_given_t(float t);

public:
    CubicBezierAnimation();
    CubicBezierAnimation(Timer *timer, float p0x, float p0y, float p1x, float p1y);

    virtual float modify_feature(float feature);
};