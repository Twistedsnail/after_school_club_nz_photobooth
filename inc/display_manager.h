#pragma once

#include "gl_base.h"
#include <GLFW/glfw3.h>
#include <Magick++.h>
#include <string>

typedef struct {
    int x;
    int y;
    unsigned width;
    unsigned height;
} rect_t;

typedef struct {
    bool textured;
    float stroke;
    float fill_colour[3];
    float stroke_colour[3];
    GLuint texture;
    rect_t rect_geom;
    void (*click_fn)();
} ui_panel_t;

void open_window(void);
void update_window(void);
bool should_run(void);
bool get_triggered(void);

extern bool show_full;
void load_preview(char *data, unsigned long *sze);
void load_texture(std::string path);
void load_full_texture(std::string path);
