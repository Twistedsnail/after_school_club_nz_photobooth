#include "interface.h"

#include <cmath>

#include "gl_base.h"

#define OMEGA   6.283185307f

Timer::Timer() {
    timer_value = timer_maximum = 0.f;
    one_shot = false;
}

Timer::Timer(float maximum, bool repeat, void (*on_complete_ptr)(void)) {
    timer_maximum = maximum;
    one_shot = !repeat;
    complete_ptr = on_complete_ptr;
}

void Timer::reset() {
    timer_value = 0.f;
}

void Timer::progress_timer(float dt) {
    if(timer_value < timer_maximum) {
        timer_value += dt;

        if(timer_value >= timer_maximum && complete_ptr != nullptr) {
            complete_ptr();
        } 

        if(!one_shot) {
            while(timer_value > timer_maximum && timer_maximum != 0.f) {
                timer_value -= timer_maximum;
            }
        }
        else {
            if(timer_value > timer_maximum) {
                timer_value = timer_maximum;
            }
        }
    }    
}

float Timer::getTimerMaximum() {
    return timer_maximum;
}

float Timer::getTimerValue() {
    return timer_value;
}

Animation::Animation() {
    timer_ptr = nullptr;
}

Animation::Animation(Timer *timer) {
    timer_ptr = timer;
}

float Animation::getTimerValue() {
    if(timer_ptr != nullptr) {
        return timer_ptr->getTimerValue();
    }
    return 0.f;
}

float Animation::getTimerMaximum() {
    if(timer_ptr != nullptr) {
        return timer_ptr->getTimerMaximum();
    }
    return 0.f;
}

float Animation::modify_feature(float feature) {
    return feature;
}

UI_Panel::UI_Panel() {
    x = y = width = height = 0.f;
}

UI_Panel::UI_Panel(float panel_x, float panel_y, float panel_width, float panel_height) {
    x = panel_x;
    y = panel_y;
    width = panel_width;
    height = panel_height;
}

bool UI_Panel::click_handler(float click_x, float click_y) {
    return false;
}

void UI_Panel::render() {
    return;
}

Texture_Panel::Texture_Panel() {
    texture_id = 0;
    click_ptr = nullptr;
}

Texture_Panel::Texture_Panel(GLuint texture, float panel_x, float panel_y, float panel_width, float panel_height,
                             void (*on_click_ptr)(void)) : UI_Panel(panel_x, panel_y, panel_width, panel_height) {
    texture_id = texture;
    click_ptr = on_click_ptr;
}

void Texture_Panel::render() {
    glUniform1i(is_textured_unif, 1);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    float pos_offset[2] = {x, y};
    float pos_scale[2] = {width, height};
    glUniform2fv(pos_offset_unif, 1, &pos_offset[0]);
    glUniform2fv(pos_scale_unif, 1, &pos_scale[0]);

    float uv_offset[2] = {0.f, 0.f};
    float uv_scale[2] = {1.f, 1.f};
    glUniform2fv(uv_offset_unif, 1, &uv_offset[0]);
    glUniform2fv(uv_scale_unif, 1, &uv_scale[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}

bool Texture_Panel::click_handler(float click_x, float click_y) {
    if(click_ptr != nullptr) {
        if(click_x >= x && click_x <= x + width && click_y >= y && click_y <= y + height) {
            click_ptr();
            return true;
        }
    }

    return false;
}

Animated_Panel::Animated_Panel() {
    columns = rows = frame = 0;
}

Animated_Panel::Animated_Panel(GLuint texture, unsigned x_tiles, unsigned y_tiles,
                                float panel_x, float panel_y, float panel_width, float panel_height,
                                void (*on_click_ptr)(void)) : Texture_Panel(texture, panel_x, panel_y, panel_width, panel_height, on_click_ptr) {
    frame = 0;
    columns = x_tiles;
    rows = y_tiles;
}

void Animated_Panel::render() {
    glUniform1i(is_textured_unif, 1);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    float pos_offset[2] = {x, y};
    float pos_scale[2] = {width, height};
    glUniform2fv(pos_offset_unif, 1, &pos_offset[0]);
    glUniform2fv(pos_scale_unif, 1, &pos_scale[0]);

    float uv_offset[2] = {float(frame % columns), float(frame / columns)};
    float uv_scale[2] = {1.f / float(columns), 1.f / float(rows)};
    glUniform2fv(uv_offset_unif, 1, &uv_offset[0]);
    glUniform2fv(uv_scale_unif, 1, &uv_scale[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}

void Animated_Panel::set_frame(unsigned new_frame) {
    if(new_frame < rows * columns) {
        frame = new_frame;
    }
}

unsigned Animated_Panel::get_max_frames() {
    return rows * columns;
}

BounceAnimation::BounceAnimation() {
    scale = offset = frequency = 0.f;
}

BounceAnimation::BounceAnimation(Timer *timer, float anim_scale, float anim_offset, float anim_frequency) : Animation(timer) {
    scale = anim_scale;
    offset = anim_offset;
    frequency = anim_frequency;
}

float BounceAnimation::modify_feature(float feature) {
    float amount = abs(scale * sin(OMEGA * frequency * getTimerValue() + offset * OMEGA));
    return -amount;
}

ScaleAnimation::ScaleAnimation() {
    scale = offset = frequency = 0.f;
}

ScaleAnimation::ScaleAnimation(Timer *timer, float anim_scale, float anim_offset, float anim_frequency) : Animation(timer) {
    scale = anim_scale;
    offset = anim_offset;
    frequency = anim_frequency;
}

float ScaleAnimation::modify_feature(float feature) {
    float amount = scale * sin(OMEGA * frequency * getTimerValue()) + offset;
    return feature * amount;
}

CubicBezierAnimation::CubicBezierAnimation() {
    x0 = x1 = y0 = y1 = 0.f;
}

CubicBezierAnimation::CubicBezierAnimation(Timer *timer, float p0x, float p0y, float p1x, float p1y) : Animation(timer) {
    x0 = p0x;
    y0 = p0y;
    x1 = p1x;
    y1 = p1y;
}

float CubicBezierAnimation::get_bezier_term(int i, float t) {
    float term = 0.f;
    switch(i) {
        case 0:
            term = 3.f * t * pow(1.f - t, 2.f);
            break;
        case 1:
            term = 3.f * (1.f - t) * pow(t, 2.f);
            break;
        case 2:
            term = pow(t, 3.f);
            break;
        default:
            break;
    }

    return term;
}

float CubicBezierAnimation::get_bezier_x_gradient(float t) {
    float dxdt = x0 * (t * (9.f * t - 12.f) + 3.f);
    dxdt += x1 * (6.f - 9.f * t) * t;
    dxdt += 3.f * pow(t, 2.f);

    return dxdt;
}

float CubicBezierAnimation::get_y_given_t(float t) {
    return (get_bezier_term(0, t) * y0 + get_bezier_term(1, t) * y1 + get_bezier_term(2, t));
}

float CubicBezierAnimation::get_x_given_t(float t) {
    return (get_bezier_term(0, t) * x0 + get_bezier_term(1, t) * x1 + get_bezier_term(2, t));
}

float CubicBezierAnimation::get_t_given_x(float x) {
    if(x == 0.f) return 0.f;

    float guess_t = x;
    float current_x_error = get_x_given_t(guess_t) - x;
    float slope;

    while(abs(current_x_error / x) >= 0.001f) {
        current_x_error = get_x_given_t(guess_t) - x;
        slope = get_bezier_x_gradient(guess_t);
        if(slope == 0.f) return guess_t;

        guess_t -= current_x_error / slope;
    }

    return guess_t;
}

float CubicBezierAnimation::modify_feature(float feature) {
    float x = getTimerValue() / getTimerMaximum();
    float t = get_t_given_x(x);
    
    return get_y_given_t(t);
}

AnimationLink::AnimationLink() {
    linked_animation = nullptr;
    linked_feature = nullptr;
    animation_scale = 1.f;
    animation_scale = 0.f;
    last_value = 0.f;
}

AnimationLink::AnimationLink(Animation *animation, float *feature, std::vector<float *> apply_list, float scale, float offset) {
    linked_animation = animation;
    linked_feature = feature;
    targets = apply_list;
    animation_scale = scale;
    animation_offset = offset;
    last_value = 0.f;
}

void AnimationLink::apply_animation() {
    if(linked_animation == nullptr) return;

    float animation_value;
    if(linked_feature == nullptr) {
        animation_value = animation_scale * linked_animation->modify_feature(0.f) + animation_offset;
    }
    else animation_value = animation_scale * linked_animation->modify_feature(*linked_feature) + animation_offset;
    last_value = animation_value;

    for(unsigned i = 0; i < targets.size(); i++) {
        *targets[i] += animation_value;
    }
}

void AnimationLink::remove_animation() {
    for(unsigned i = 0; i < targets.size(); i++) {
        *targets[i] -= last_value;
    }
}