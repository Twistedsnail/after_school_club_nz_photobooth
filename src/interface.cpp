#include "interface.h"

#include <cmath>

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