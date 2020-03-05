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
static GLuint preview_tex, blurred_tex, touch_text_tex, select_back_tex, select_vertical_tex, select_polaroid_tex, capture_background_tex;

static state_t state = IDLE_STATE;

static Magick::Geometry window_dimensions;

static std::vector<Texture_Panel> ui_panels[NUMBER_OF_STATES];

static int selected = -1;

static Timer bounce_timer = Timer(3.f);
static BounceAnimation bounce1 = BounceAnimation(&bounce_timer, 5.f, 0.f, 0.5f), bounce2 = BounceAnimation(&bounce_timer, 5.f, 0.25f, 0.5f);
static ScaleAnimation scale1 = ScaleAnimation(&bounce_timer, 0.05f, 0.f, 1.f);

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
            UI_Panel *panel = &ui_panels[state][i];

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
    state = CAPTURE_STATE;
    selected = 2;
}

static void set_green() {
    state = CAPTURE_STATE;
    selected = 1;
}

static void goto_layout_state() {
    state = SELECT_STATE;
    bounce_timer.reset();
}

/*static void render_panel(ui_panel_t *panel) {
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
}*/

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
}

static void load_interface() {
    load_panel_textures();

    Texture_Panel idle_background_panel = Texture_Panel(blurred_tex, 0.f, -26.f, 1280.f, 853.f, goto_layout_state);
    Texture_Panel idle_text_panel = Texture_Panel(touch_text_tex, 115.f, 332.f, 1051.f, 136.f);

    ui_panels[IDLE_STATE].push_back(idle_background_panel);
    ui_panels[IDLE_STATE].push_back(idle_text_panel);

    Texture_Panel select_background_panel = Texture_Panel(select_back_tex, 0.f, 0.f, 1280.f, 800.f, set_red);
    Texture_Panel select_vertical_panel = Texture_Panel(select_vertical_tex, 216.f, 109.f, 359.f, 606.f, set_green);
    Texture_Panel select_polaroid_panel = Texture_Panel(select_polaroid_tex, 684.f, 182.f, 399.f, 461.f, set_blue);

    ui_panels[SELECT_STATE].push_back(select_background_panel);
    ui_panels[SELECT_STATE].push_back(select_vertical_panel);
    ui_panels[SELECT_STATE].push_back(select_polaroid_panel);

    Texture_Panel capture_background_panel = Texture_Panel(capture_background_tex, 0.f, 0.f, 1280.f, 800.f);
    Texture_Panel capture_preview_panel = Texture_Panel(preview_tex, 280.f, 100.f, 900.f, 600.f);

    ui_panels[CAPTURE_STATE].push_back(capture_background_panel);
    ui_panels[CAPTURE_STATE].push_back(capture_preview_panel);
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

    Timer test_tim = Timer(1.f);
    CubicBezierAnimation test_bezier = CubicBezierAnimation(&test_tim, 0.f, 1.f, 0.f, 1.f);

    printf("\nCubic Test: \n");
    for(int i = 0; i < 1001; i ++) {
        printf("%.3f\n", test_bezier.modify_feature(0.f));
        test_tim.progress_timer(0.001f);
    }

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

    bounce_timer.progress_timer(dT);

    for(unsigned i = 0; i < ui_panels[state].size(); i ++) {
        UI_Panel *panel = &ui_panels[state][i];

        if(state == IDLE_STATE && i == 1) {            
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
        else if(state == SELECT_STATE) {
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
        else panel->render();
    }

    glfwSwapBuffers(window);
}

bool should_run() {
    return !glfwWindowShouldClose(window);
}
