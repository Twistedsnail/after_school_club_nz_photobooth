#include "display_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <GLFW/glfw3.h>
#include <Magick++.h>

#include "gl_base.h"
#include "statemachine.h"

static GLFWwindow *window;
static Magick::Geometry window_dimensions;

static void glfw_error(int code, const char *msg) {
	printf("GLFW Error %i: %s", code, msg);
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

        check_click(x_pos, y_pos);
    }
}

void open_window() {
	glfwInit();
	glfwSetErrorCallback(glfw_error);

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);

	window = glfwCreateWindow(mode->width, mode->height, "After School Club", monitor, NULL);
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
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    run_state(dT);

    glfwSwapBuffers(window);
}

bool should_run() {
    return !glfwWindowShouldClose(window);
}
