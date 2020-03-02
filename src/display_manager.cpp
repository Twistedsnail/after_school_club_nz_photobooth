#include "display_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <GLFW/glfw3.h>
#include <Magick++.h>
#include <vector>

#include "gl_base.h"
#include "interface.h"

static GLFWwindow *window;
static GLuint preview_tex, blurred_tex, touch_text_tex, select_back_tex, select_vertical_tex, select_polaroid_tex;

static unsigned state = 0;

static Magick::Geometry window_dimensions;

static std::vector<ui_panel_t> ui_panels[2];

static int selected = 0;

static BounceAnimation bounce1 = BounceAnimation(3.f, 10.f, 0.f, 1.f), bounce2 = BounceAnimation(3.f, 20.f, 0.f, 5.f);
static ScaleAnimation scale1 = ScaleAnimation(3.f, 0.05f, 0.f, 1.f);

static void glfw_error(int code, const char *msg) {
	printf("GLFW Error %i: %s", code, msg);
}

static rect_t make_rect(int x, int y, unsigned width, unsigned height) {
    rect_t rect = {
        .x = x,
        .y = y,
        .width = width,
        .height = height
    };

    return rect;
}

void load_preview(char *data, unsigned long *sze) {
    Magick::Blob data_blob(data, *sze);
    Magick::Image eye_img(data_blob);
    const Magick::PixelPacket *ppp = eye_img.getConstPixels(0, 0, eye_img.columns(), eye_img.rows());
    glBindTexture(GL_TEXTURE_2D, preview_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, eye_img.columns(), eye_img.rows(), 0, GL_BGRA, GL_UNSIGNED_SHORT, ppp);
}

void load_texture(std::string path) {
    Magick::Image eye_img(path.c_str());
    const Magick::PixelPacket *ppp = eye_img.getConstPixels(0, 0, eye_img.columns(), eye_img.rows());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, eye_img.columns(), eye_img.rows(), 0, GL_BGRA, GL_UNSIGNED_SHORT, ppp);
}

static void load_blurred_texture(std::string path) {
    Magick::Image eye_img(path.c_str());
    eye_img.blur(100.0, 100.0);
    eye_img.syncPixels();
    const Magick::PixelPacket *ppp = eye_img.getConstPixels(0, 0, eye_img.columns(), eye_img.rows());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, eye_img.columns(), eye_img.rows(), 0, GL_BGRA, GL_UNSIGNED_SHORT, ppp);
}

void load_full_texture(std::string path) {
    Magick::Image eye_img(path.c_str());
    const Magick::PixelPacket *ppp = eye_img.getConstPixels(0, 0, eye_img.columns(), eye_img.rows());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, eye_img.columns(), eye_img.rows(), 0, GL_BGRA, GL_UNSIGNED_SHORT, ppp);
}

static void resize_graphics(GLFWwindow *window, int width, int height) {
    int x_off = 0, y_off = 0;
    if(width > 1280) x_off = (width - 1280)/2;
    if(height > 800) y_off = (height - 800)/2;

    window_dimensions.width(1280);
    window_dimensions.height(800);
    window_dimensions.xOff(x_off);
    window_dimensions.yOff(y_off);

    glViewport(x_off, y_off, 1280, 800);
}

static void click_callback(GLFWwindow *window, int button, int action, int modifiers) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        double x_pos, y_pos;
        glfwGetCursorPos(window, &x_pos, &y_pos);
        x_pos -= (double)window_dimensions.xOff();
        y_pos -= (double)window_dimensions.yOff();

        bool clicked = false;

        for(unsigned i = 0; i < ui_panels[state].size(); i ++) {
            ui_panel_t *panel = &ui_panels[state][i];

            if(panel->click_fn == nullptr) continue;

            if(panel->rect_geom.x <= (int)x_pos && panel->rect_geom.y <= (int)y_pos &&
            (panel->rect_geom.x + (int)panel->rect_geom.width) >= (int)x_pos &&
            (panel->rect_geom.y + (int)panel->rect_geom.height) >= (int)y_pos) {
                clicked = true;
                panel->click_fn();
            }
        }

        if(!clicked) {
            glClearColor(0.f, 0.f, 0.f, 1.f);
        }
    }
}

static void set_red() {
    selected = 0;
}

static void set_blue() {
    selected = 2;
    bounce1.reset();
    bounce2.reset();
}

static void set_green() {
    selected = 1;
    bounce1.reset();
    bounce2.reset();
}

static void goto_layout_state() {
    state = 1;
}

static void render_panel(ui_panel_t *panel) {
    glUniform1i(is_textured_unif, panel->textured ? 1 : -1);
    glBindTexture(GL_TEXTURE_2D, panel->texture);

    float offset[2] = {(float)panel->rect_geom.x / (float)window_dimensions.width(),
                        (float)panel->rect_geom.y / (float)window_dimensions.height()};
    float scale[2] = {(float)panel->rect_geom.width / (float)window_dimensions.width(),
                        (float)panel->rect_geom.height / (float)window_dimensions.height()};

    glUniform3fv(fill_colour_unif, 1, &panel->fill_colour[0]);
    glUniform2fv(offset_unif, 1, &offset[0]);
    glUniform2fv(scale_unif, 1, &scale[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

    if(panel->stroke > 0.f) {
        glLineWidth(panel->stroke);
        glUniform3fv(fill_colour_unif, 1, &panel->stroke_colour[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_buffer);
        glDrawElements(GL_LINE_STRIP, 5, GL_UNSIGNED_SHORT, NULL);
    }
}

static void load_panel_textures() {
    create_texture(&preview_tex);
    load_texture("../data/prev.jpg");

    create_texture(&blurred_tex);
    load_blurred_texture("../data/prev.jpg");

    create_texture(&touch_text_tex);
    load_texture("../data/touch_text.png");

    create_texture(&select_back_tex);
    load_texture("../data/select_background.png");

    create_texture(&select_vertical_tex);
    load_texture("../data/select_vertical.png");

    create_texture(&select_polaroid_tex);
    load_texture("../data/select_polaroid.png");
}

void open_window() {
	glfwInit();
	glfwSetErrorCallback(glfw_error);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);

	window = glfwCreateWindow(mode->width, mode->height, "Test", monitor, NULL);
	glfwMakeContextCurrent(window);

	glewInit();

	glfwSwapInterval(0);

	int win_width, win_height;
	glfwGetFramebufferSize(window, &win_width, &win_height);
	resize_graphics(window, win_width, win_height);

	glfwSetFramebufferSizeCallback(window, resize_graphics);
	glfwSetMouseButtonCallback(window, click_callback);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	init_gl();
    load_panel_textures();

    // Idle background
    ui_panel_t touch_panel1 = {
        .textured = true,
        .stroke = 0.f,
        .fill_colour = {0.f, 0.f, 0.f},
        .stroke_colour = {0.f, 0.f, 0.f},
        .texture = blurred_tex,
        .rect_geom = make_rect(0, -26, 1280, 853),
        .click_fn = goto_layout_state
    };

    // Idle text
    ui_panel_t touch_panel2 = {
        .textured = true,
        .stroke = 0.f,
        .fill_colour = {0.f, 0.f, 0.f},
        .stroke_colour = {0.f, 0.f, 0.f},
        .texture = touch_text_tex,
        .rect_geom = make_rect(window_dimensions.width() / 2 - 510, window_dimensions.height() / 2 - 71, 1021, 142),
        .click_fn = nullptr
    };

    // Select background
    ui_panel_t test_panel1 = {
        .textured = true,
        .stroke = 0.f,
        .fill_colour = {1.f, 0.f, 0.f},
        .stroke_colour = {0.f, 1.f, 1.f},
        .texture = select_back_tex,
        .rect_geom = make_rect(0, 0, window_dimensions.width(), window_dimensions.height()),
        .click_fn = set_red
    };

    // Select vertical
    ui_panel_t test_panel2 = {
        .textured = true,
        .stroke = 0.f,
        .fill_colour = {0.f, 1.f, 0.f},
        .stroke_colour = {1.f, 0.f, 1.f},
        .texture = select_vertical_tex,
        .rect_geom = make_rect(216, 109, 359, 606),
        .click_fn = set_green
    };

    // Select polaroid
    ui_panel_t test_panel3 = {
        .textured = true,
        .stroke = 0.f,
        .fill_colour = {0.f, 0.f, 1.f},
        .stroke_colour = {1.f, 1.f, 0.f},
        .texture = select_polaroid_tex,
        .rect_geom = make_rect(684, 182, 399, 461),
        .click_fn = set_blue
    };

    ui_panels[0].push_back(touch_panel1);
    ui_panels[0].push_back(touch_panel2);

    ui_panels[1].push_back(test_panel1);
    ui_panels[1].push_back(test_panel2);
    ui_panels[1].push_back(test_panel3);
}

void update_window() {
    static double old_t = glfwGetTime(), t = glfwGetTime();
    static double dT = 0;

    t = glfwGetTime();
    dT = t - old_t;
    old_t = t;

    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(unsigned i = 0; i < ui_panels[state].size(); i ++) {
        ui_panel_t *panel = &ui_panels[state][i];

        if(state == 0 && i == 1) {
            scale1.progress_timer(dT);
            
            int xAmount = (int)scale1.modify_feature((float)panel->rect_geom.width);
            int yAmount = (int)scale1.modify_feature((float)panel->rect_geom.height);

            panel->rect_geom.y -= yAmount / 2;
            panel->rect_geom.height += yAmount;
            panel->rect_geom.x -= xAmount / 2;
            panel->rect_geom.width += xAmount;

            render_panel(panel);

            panel->rect_geom.y += yAmount / 2;
            panel->rect_geom.height -= yAmount;
            panel->rect_geom.x += xAmount / 2;
            panel->rect_geom.width -= xAmount;
        }
        else if(state == 1) {
            if(selected == 1 && i == 1) {
                bounce1.progress_timer(dT);
                int original_y = panel->rect_geom.y;
                panel->rect_geom.y = (int)bounce1.modify_feature((float)panel->rect_geom.y);
                render_panel(panel);
                panel->rect_geom.y = original_y;
            }
            else if(selected == 2 && i == 2) {
                bounce2.progress_timer(dT);
                int original_y = panel->rect_geom.y;
                panel->rect_geom.y = (int)bounce2.modify_feature((float)panel->rect_geom.y);
                render_panel(panel);
                panel->rect_geom.y = original_y;
            }
            else render_panel(panel);
        }
        else render_panel(panel);
    }

    glfwSwapBuffers(window);
}

bool should_run() {
    return !glfwWindowShouldClose(window);
}
