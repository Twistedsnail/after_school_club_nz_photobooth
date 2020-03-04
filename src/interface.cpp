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

Texture_Panel::Texture_Panel(GLuint texture, float panel_x, float panel_y, float panel_width, float panel_height, void (*on_click_ptr)(void)) : UI_Panel(panel_x, panel_y, panel_width, panel_height) {
    texture_id = texture;
    click_ptr = on_click_ptr;
}

void Texture_Panel::render() {
    glUniform1i(is_textured_unif, 1);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    float offset[2] = {x, y};
    float scale[2] = {width, height};

    glUniform2fv(offset_unif, 1, &offset[0]);
    glUniform2fv(scale_unif, 1, &scale[0]);

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
    return feature - amount;
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