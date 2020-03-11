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
static GLuint preview_tex, blurred_tex, touch_text_tex, select_back_tex, select_vertical_tex, select_polaroid_tex, capture_background_tex, timer_tex, counter_tex;

static state_t state = IDLE_STATE;

static Magick::Geometry window_dimensions;

static std::vector<UI_Panel *> ui_panels[NUMBER_OF_STATES];

static int selected = 0;
static unsigned countdown = 5;
static bool counting = true;

static void decrement_counter() {
    countdown --;
    if(countdown == 1) {
        counting = false;
    }
}

static Timer bounce_timer = Timer(3.f), dial_timer = Timer(1.f, true, decrement_counter);
static BounceAnimation bounce1 = BounceAnimation(&bounce_timer, 5.f, 0.f, 0.5f), bounce2 = BounceAnimation(&bounce_timer, 5.f, 0.25f, 0.5f);
static ScaleAnimation scale1 = ScaleAnimation(&bounce_timer, 0.05f, 0.f, 1.f);

static Timer tween_timer = Timer(0.5f, false), second_tween_timer = Timer(0.5f, false);
static CubicBezierAnimation in_out_bezier = CubicBezierAnimation(&tween_timer, 0.42f, 0.f, 0.58f, 1.f), out_bezier = CubicBezierAnimation(&second_tween_timer, 0.0f, 0.f, 0.58f, 1.f);

static bool exiting = false;

static void glfw_error(int code, const char *msg) {
	printf("GLFW Error %i: %s", code, msg);
}

static const void *get_img_pixels(Magick::Image img) {
    return img.getConstPixels(0, 0, img.columns(), img.rows());
}

void load_preview(char *data, unsigned long *sze) {
    Magick::Blob data_blob(data, *sze);
    Magick::Image img(data_blob);

    glBindTexture(GL_TEXTURE_2D, preview_tex);
    load_pixels(img.columns(), img.rows(), get_img_pixels(img));
}

void load_texture(std::string path) {
    Magick::Image img(path.c_str());
    load_pixels(img.columns(), img.rows(), get_img_pixels(img));
}

static void load_blurred_texture(std::string path) {
    Magick::Image img(path.c_str());
    img.blur(100.0, 100.0);
    img.syncPixels();

    load_pixels(img.columns(), img.rows(), get_img_pixels(img));
}

static void resize_graphics(GLFWwindow *window, int width, int height) {
    int x_off = 0, y_off = 0;
    if(width > 1280) x_off = (width - 1280)/2;
    if(height > 800) y_off = (height - 800)/2;

    window_dimensions = Magick::Geometry(1280, 800, x_off, y_off);

    glUniform1f(width_unif, 1280.f);
    glUniform1f(height_unif, 800.f);

    glViewport(x_off, y_off, 1280, 800);
}

static void click_callback(GLFWwindow *window, int button, int action, int modifiers) {
    static bool primed = false;

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        primed = true;
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && primed) {
        primed = false;

        double x_pos, y_pos;
        glfwGetCursorPos(window, &x_pos, &y_pos);
        x_pos -= (double)window_dimensions.xOff();
        y_pos -= (double)window_dimensions.yOff();

        for(int i = ui_panels[state].size() - 1; i > -1; i --) {
            UI_Panel *panel = ui_panels[state][i];

            if(panel->click_handler(x_pos, y_pos)) {
                break;
            }
        }
    }
}

static void set_red() {
    selected = 0;
}

static void set_blue() {
    exiting = true;
    selected = 2;
}

static void set_green() {
    exiting = true;
    selected = 1;
}

static void goto_layout_state() {
    exiting = true;
}

static void do_idle_state(float dT) {
    bounce_timer.progress_timer(dT);

    if(exiting) {
        if(tween_timer.getTimerValue() == tween_timer.getTimerMaximum()) {
            exiting = false;
            tween_timer.reset();
            state = SELECT_STATE;
            return;
        }

        tween_timer.progress_timer(dT);
        float tween = 800.f * in_out_bezier.modify_feature(0.f);

        for(int i = 0; i < (int)ui_panels[IDLE_STATE].size(); i ++) {
            UI_Panel *panel = ui_panels[IDLE_STATE][i];

            if(i == 1) {            
                float xAmount = scale1.modify_feature(panel->width);
                float yAmount = scale1.modify_feature(panel->height);

                panel->y -= yAmount / 2.f + tween;
                panel->height += yAmount;
                panel->x -= xAmount / 2.f;
                panel->width += xAmount;

                panel->render();

                panel->y += yAmount / 2.f + tween;
                panel->height -= yAmount;
                panel->x += xAmount / 2.f;
                panel->width -= xAmount;
            }
            else {
                float original_y = panel->y;
                
                panel->y -= tween;
                panel->render();
                panel->y = original_y;
            }
        }

        for(int i = 0; i < (int)ui_panels[SELECT_STATE].size(); i ++) {
            UI_Panel *panel = ui_panels[SELECT_STATE][i];

            if(i == 1) {
                float original_y = panel->y;

                panel->y = bounce1.modify_feature(panel->y) + (800.f - tween);
                panel->render();
                panel->y = original_y;
            }
            else if(i == 2) {
                float original_y = panel->y;

                panel->y = bounce2.modify_feature(panel->y) + (800.f - tween);
                panel->render();
                panel->y = original_y;
            }
            else {
                float original_y = panel->y;
                
                panel->y += (800.f - tween);
                panel->render();
                panel->y = original_y;
            }
        }
    }
    else {
        for(int i = 0; i < (int)ui_panels[IDLE_STATE].size(); i ++) {
            UI_Panel *panel = ui_panels[IDLE_STATE][i];

            if(i == 1) {            
                float xAmount = scale1.modify_feature(panel->width);
                float yAmount = scale1.modify_feature(panel->height);

                panel->y -= yAmount / 2.f;
                panel->height += yAmount;
                panel->x -= xAmount / 2.f;
                panel->width += xAmount;

                panel->render();

                panel->y += yAmount / 2.f;
                panel->height -= yAmount;
                panel->x += xAmount / 2.f;
                panel->width -= xAmount;
            }
            else panel->render();
        }
    }
}

static void do_select_state(float dT) {
    bounce_timer.progress_timer(dT);

    static bool exiting_stage_2 = false;

    if(exiting) {
        if(second_tween_timer.getTimerValue() == second_tween_timer.getTimerMaximum()) {
            exiting_stage_2 = true;
        }

        second_tween_timer.progress_timer(dT);

        if(exiting_stage_2) {
            if(tween_timer.getTimerValue() == tween_timer.getTimerMaximum()) {
                exiting = false;
                tween_timer.reset();
                state = CAPTURE_STATE;
                return;
            }

            tween_timer.progress_timer(dT);

            float tween = 1280.f * in_out_bezier.modify_feature(0.f);

            for(int i = 0; i < (int)ui_panels[CAPTURE_STATE].size(); i ++) {
                UI_Panel *panel = ui_panels[CAPTURE_STATE][i];

                float original_x = panel->x;
                
                panel->x += (1280.f - tween);
                panel->render();
                panel->x = original_x;
            }

            for(int i = 0; i < (int)ui_panels[SELECT_STATE].size(); i ++) {
                UI_Panel *panel = ui_panels[SELECT_STATE][i];

                if(i == 0) {
                    float original_x = panel->x;

                    panel->x -= tween;
                    panel->render();
                    panel->x = original_x;
                }
                else if(i == selected) {
                    float original_y = panel->y;
                    float original_x = panel->x;

                    panel->y = bounce2.modify_feature(panel->y);
                    panel->x -= tween;
                    panel->render();
                    panel->y = original_y;
                    panel->x = original_x;
                }
            }
        }
        else {
            for(int i = 0; i < (int)ui_panels[SELECT_STATE].size(); i ++) {
                UI_Panel *panel = ui_panels[SELECT_STATE][i];

                if(i == 0) {
                    panel->render();
                }
                else if(i == selected) {
                    float original_y = panel->y;

                    if(selected == 1) {
                        panel->y = bounce1.modify_feature(panel->y);
                    }
                    else {
                        panel->y = bounce2.modify_feature(panel->y);
                    }
                    
                    panel->render();
                    panel->y = original_y;
                }
                else {
                    float original_y = panel->y;

                    panel->y = bounce2.modify_feature(panel->y) + 800.f * out_bezier.modify_feature(0.f);
                    panel->render();
                    panel->y = original_y;
                }
            }
        }
    }
    else {
        for(int i = 0; i < (int)ui_panels[SELECT_STATE].size(); i ++) {
            UI_Panel *panel = ui_panels[SELECT_STATE][i];

            if(i == 1) {
                float original_y = panel->y;

                panel->y = bounce1.modify_feature(panel->y);
                panel->render();
                panel->y = original_y;
            }
            else if(i == 2) {
                float original_y = panel->y;

                panel->y = bounce2.modify_feature(panel->y);
                panel->render();
                panel->y = original_y;
            }
            else panel->render();
        }
    }
}

static void do_capture_state(float dT) {
    for(unsigned i = 0; i < ui_panels[CAPTURE_STATE].size(); i ++) {
        UI_Panel *panel = ui_panels[CAPTURE_STATE][i];

        if(i == 2) {
            if(counting) {
                dial_timer.progress_timer(dT);
                float frame = (float)((Animated_Panel *)panel)->get_max_frames() * dial_timer.getTimerValue() / dial_timer.getTimerMaximum();
                ((Animated_Panel *)panel)->set_frame(frame);
            }
        }
        else if(i == 3) {
            float frame = 5.f - ((float)countdown - 1.f);
            ((Animated_Panel *)panel)->set_frame(frame);
        }

        panel->render();
    }
}

static void load_panel_textures() {
    create_texture(&preview_tex);
    load_texture("../data/prev.jpg");

    create_texture(&blurred_tex);
    load_blurred_texture("../data/prev.jpg");

    create_texture(&touch_text_tex);
    load_texture("../data/touch_text_fixed.png");

    create_texture(&select_back_tex);
    load_texture("../data/select_background.png");

    create_texture(&select_vertical_tex);
    load_texture("../data/select_vertical.png");

    create_texture(&select_polaroid_tex);
    load_texture("../data/select_polaroid.png");

    create_texture(&capture_background_tex);
    load_texture("../data/capture_background.png");

    create_texture(&timer_tex);
    load_texture("../data/dial_tileset.png");

    create_texture(&counter_tex);
    load_texture("../data/countdown_tileset.png");
}

static void load_interface() {
    load_panel_textures();

    UI_Panel *idle_background_panel = new Texture_Panel(blurred_tex, 0.f, -26.f, 1280.f, 853.f, goto_layout_state);
    UI_Panel *idle_text_panel = new Texture_Panel(touch_text_tex, 115.f, 332.f, 1051.f, 136.f);

    ui_panels[IDLE_STATE].push_back(idle_background_panel);
    ui_panels[IDLE_STATE].push_back(idle_text_panel);

    UI_Panel *select_background_panel = new Texture_Panel(select_back_tex, 0.f, 0.f, 1280.f, 800.f, set_red);
    UI_Panel *select_vertical_panel = new Texture_Panel(select_vertical_tex, 216.f, 109.f, 359.f, 606.f, set_green);
    UI_Panel *select_polaroid_panel = new Texture_Panel(select_polaroid_tex, 684.f, 182.f, 399.f, 461.f, set_blue);

    ui_panels[SELECT_STATE].push_back(select_background_panel);
    ui_panels[SELECT_STATE].push_back(select_vertical_panel);
    ui_panels[SELECT_STATE].push_back(select_polaroid_panel);

    UI_Panel *capture_background_panel = new Texture_Panel(capture_background_tex, 0.f, 0.f, 1280.f, 800.f);
    UI_Panel *capture_preview_panel = new Texture_Panel(preview_tex, 320.f, 100.f, 900.f, 600.f);
    UI_Panel *timer_panel = new Animated_Panel(timer_tex, 10, 10, 30.f, 275.f, 250.f, 250.f);
    UI_Panel *counter_panel = new Animated_Panel(counter_tex, 5, 1, 30.f, 275.f, 250.f, 250.f);

    ui_panels[CAPTURE_STATE].push_back(capture_background_panel);
    ui_panels[CAPTURE_STATE].push_back(capture_preview_panel);
    ui_panels[CAPTURE_STATE].push_back(timer_panel);
    ui_panels[CAPTURE_STATE].push_back(counter_panel);
}

void open_window() {
	glfwInit();
	glfwSetErrorCallback(glfw_error);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);

	window = glfwCreateWindow(mode->width, mode->height, "Test", monitor, NULL);
	glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

	glewInit();
    init_gl();

	int win_width, win_height;
    glfwSetFramebufferSizeCallback(window, resize_graphics);
	glfwGetFramebufferSize(window, &win_width, &win_height);
	resize_graphics(window, win_width, win_height);

	glfwSetMouseButtonCallback(window, click_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    load_interface();
}

void update_window() {
    static double old_t = glfwGetTime(), t = glfwGetTime();
    static double dT = 0;

    t = glfwGetTime();
    dT = t - old_t;
    old_t = t;

    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(state == IDLE_STATE) {
        do_idle_state(dT);
    }
    else if(state == SELECT_STATE) {
        do_select_state(dT);
    }
    else if(state == CAPTURE_STATE) {
        do_capture_state(dT);
    }

    glfwSwapBuffers(window);
}

bool should_run() {
    return !glfwWindowShouldClose(window);
}
