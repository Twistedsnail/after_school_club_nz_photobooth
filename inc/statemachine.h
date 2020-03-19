#pragma once

typedef enum {
    IDLE_STATE,
    SELECT_STATE,
    CAPTURE_STATE,
    NUMBER_OF_STATES
} state_t;

state_t state = IDLE_STATE;

void load_interface();
void run_state(float dT);

void check_click(float click_x, float click_y);