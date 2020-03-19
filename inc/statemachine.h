#pragma once

#include <Magick++.h>

typedef enum {
    IDLE_STATE,
    SELECT_STATE,
    CAPTURE_STATE,
    NUMBER_OF_STATES
} state_t;

extern state_t state;

void load_interface();
void run_state(float dT);

void check_click(float click_x, float click_y);

void load_preview(char *data, unsigned long *sze);
void load_blurred_texture(char *data, unsigned long *sze);
