#include "interface.h"

#include <cmath>

#include "gl_base.h"

#define OMEGA   6.283185307f

Animation::Animation() {
    timer_value = timer_maximum = 0.f;
}

Animation::Animation(float maximum) {
    timer_maximum = maximum;
}

void Animation::reset() {
    timer_value = 0.f;
}

void Animation::progress_timer(float dt) {
    timer_value += dt;
    while(timer_value > timer_maximum && timer_maximum != 0.f) {
        timer_value -= timer_maximum;
    }
}

float Animation::modify_feature(float feature) {
    return feature;
}

float Animation::getTimerMaximum() {
    return timer_maximum;
}

float Animation::getTimerValue() {
    return timer_value;
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

BounceAnimation::BounceAnimation(float maximum, float anim_scale, float anim_offset, float anim_frequency) : Animation(maximum) {
    scale = anim_scale;
    offset = anim_offset;
    frequency = anim_frequency;
}

float BounceAnimation::modify_feature(float feature) {
    float amount = abs(scale * sin(OMEGA * frequency * getTimerValue()/getTimerMaximum())) + offset;
    return feature - amount;
}

ScaleAnimation::ScaleAnimation() {
    scale = offset = frequency = 0.f;
}

ScaleAnimation::ScaleAnimation(float maximum, float anim_scale, float anim_offset, float anim_frequency) : Animation(maximum) {
    scale = anim_scale;
    offset = anim_offset;
    frequency = anim_frequency;
}

float ScaleAnimation::modify_feature(float feature) {
    float amount = scale * sin(OMEGA * frequency * getTimerValue()/getTimerMaximum()) + offset;
    return feature * amount;
}