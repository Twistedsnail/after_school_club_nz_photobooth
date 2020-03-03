#pragma once

#include "gl_base.h"
#include <GLFW/glfw3.h>
#include <Magick++.h>
#include <string>

void open_window(void);
void update_window(void);
bool should_run(void);
bool get_triggered(void);

extern bool show_full;
void load_preview(char *data, unsigned long *sze);
void load_texture(std::string path);