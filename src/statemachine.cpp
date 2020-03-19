#include "statemachine.h"

#include <Magick++.h>
#include <vector>

#include "gl_base.h"
#include "interface.h"

static GLuint preview_tex, blurred_tex, touch_text_tex, select_back_tex, select_vertical_tex, select_polaroid_tex, capture_background_tex, timer_tex, counter_tex;

static std::vector<UI_Panel *> ui_panels[NUMBER_OF_STATES];
static std::vector<AnimationLink> animation_links;

static int selected = 0;
static unsigned countdown = 5;
static bool counting = true;

static void decrement_counter() {
    if(counting) {
        countdown --;
    }
    if(countdown == 0) {
        counting = false;
    }
}

static Timer bounce_timer = Timer(3.f), dial_timer = Timer(1.f, true, decrement_counter);
static BounceAnimation bounce1 = BounceAnimation(&bounce_timer, 5.f, 0.f, 0.5f), bounce2 = BounceAnimation(&bounce_timer, 5.f, 0.25f, 0.5f);
static ScaleAnimation scale1 = ScaleAnimation(&bounce_timer, 0.05f, 0.f, 1.f);

static Timer tween_timer = Timer(0.5f, false), second_tween_timer = Timer(0.5f, false);
static CubicBezierAnimation in_out_bezier = CubicBezierAnimation(&tween_timer, 0.42f, 0.f, 0.58f, 1.f), out_bezier = CubicBezierAnimation(&second_tween_timer, 0.0f, 0.f, 0.58f, 1.f);

static bool exiting = false;

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

void check_click(float click_x, float click_y) {
    for(int i = ui_panels[state].size() - 1; i > -1; i --) {
        UI_Panel *panel = ui_panels[state][i];

        if(panel->click_handler(click_x, click_y)) {
            break;
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

static void animate() {
    for(unsigned i = 0; i < animation_links.size(); i++) {
        animation_links[i].apply_animation();
    }
}

static void unanimate() {
    for(unsigned i = 0; i < animation_links.size(); i++) {
        animation_links[i].remove_animation();
    }
}

static void do_idle_state(float dT) {
    bounce_timer.progress_timer(dT);

    std::vector<float *> width_scale_targs = {&ui_panels[IDLE_STATE][1]->width};
    std::vector<float *> width_offset_targs = {&ui_panels[IDLE_STATE][1]->x};
    AnimationLink width_scale_link = AnimationLink((Animation *)&scale1, &ui_panels[IDLE_STATE][1]->width, width_scale_targs);
    AnimationLink width_offset_link = AnimationLink((Animation *)&scale1, &ui_panels[IDLE_STATE][1]->width, width_offset_targs, -0.5f);
    animation_links.push_back(width_scale_link);
    animation_links.push_back(width_offset_link);

    std::vector<float *> height_scale_targs = {&ui_panels[IDLE_STATE][1]->height};
    std::vector<float *> height_offset_targs = {&ui_panels[IDLE_STATE][1]->y};
    AnimationLink height_scale_link = AnimationLink((Animation *)&scale1, &ui_panels[IDLE_STATE][1]->height, height_scale_targs);
    AnimationLink height_offset_link = AnimationLink((Animation *)&scale1, &ui_panels[IDLE_STATE][1]->height, height_offset_targs, -0.5f);
    animation_links.push_back(height_scale_link);
    animation_links.push_back(height_offset_link);

    if(exiting) {
        if(tween_timer.getTimerValue() == tween_timer.getTimerMaximum()) {
            exiting = false;
            tween_timer.reset();
            state = SELECT_STATE;
            return;
        }

        tween_timer.progress_timer(dT);

        std::vector<float *> tween_targs = {&ui_panels[IDLE_STATE][0]->y, &ui_panels[IDLE_STATE][1]->y};
        AnimationLink tween_link = AnimationLink(&in_out_bezier, nullptr, tween_targs, -800.f);
        animation_links.push_back(tween_link);

        std::vector<float *> negative_tween_targs = {&ui_panels[SELECT_STATE][0]->y, &ui_panels[SELECT_STATE][1]->y, &ui_panels[SELECT_STATE][2]->y};
        AnimationLink negative_tween_link = AnimationLink(&in_out_bezier, nullptr, negative_tween_targs, -800.f, 800.f);
        animation_links.push_back(negative_tween_link);

        std::vector<float *> bounce1_targs = {&ui_panels[SELECT_STATE][1]->y};
        AnimationLink bounce1_link = AnimationLink(&bounce1, &ui_panels[SELECT_STATE][1]->y, bounce1_targs);
        animation_links.push_back(bounce1_link);

        std::vector<float *> bounce2_targs = {&ui_panels[SELECT_STATE][2]->y};
        AnimationLink bounce2_link = AnimationLink(&bounce2, &ui_panels[SELECT_STATE][2]->y, bounce2_targs);
        animation_links.push_back(bounce2_link);
    }

    animate();
    for(int i = 0; i < (int)ui_panels[IDLE_STATE].size(); i ++) {
        UI_Panel *panel = ui_panels[IDLE_STATE][i];
        panel->render();
    }
    if(exiting) {
        for(int i = 0; i < (int)ui_panels[SELECT_STATE].size(); i ++) {
            UI_Panel *panel = ui_panels[SELECT_STATE][i];
            panel->render();
        }
    }
}

static void do_select_state(float dT) {
    bounce_timer.progress_timer(dT);

    static bool exiting_stage_2 = false;

    std::vector<float *> bounce1_targs = {&ui_panels[SELECT_STATE][1]->y};
    AnimationLink bounce1_link = AnimationLink(&bounce1, &ui_panels[SELECT_STATE][1]->y, bounce1_targs);
    animation_links.push_back(bounce1_link);

    std::vector<float *> bounce2_targs = {&ui_panels[SELECT_STATE][2]->y};
    AnimationLink bounce2_link = AnimationLink(&bounce2, &ui_panels[SELECT_STATE][2]->y, bounce2_targs);
    animation_links.push_back(bounce2_link);

    if(exiting) {
        if(second_tween_timer.getTimerValue() == second_tween_timer.getTimerMaximum()) {
            exiting_stage_2 = true;
        }

        second_tween_timer.progress_timer(dT);

        std::vector<float *> drop_targs = {&ui_panels[SELECT_STATE][3 - selected]->y};
        AnimationLink drop_link = AnimationLink(&out_bezier, nullptr, drop_targs, 800.f);
        animation_links.push_back(drop_link);

        if(exiting_stage_2) {
            if(tween_timer.getTimerValue() == tween_timer.getTimerMaximum()) {
                exiting = false;
                tween_timer.reset();
                state = CAPTURE_STATE;
                return;
            }

            tween_timer.progress_timer(dT);

            std::vector<float *> tween_targs = {&ui_panels[SELECT_STATE][0]->x, &ui_panels[SELECT_STATE][1]->x, &ui_panels[SELECT_STATE][2]->x};
            AnimationLink tween_link = AnimationLink(&in_out_bezier, nullptr, tween_targs, -1280.f);
            animation_links.push_back(tween_link);

            std::vector<float *> negative_tween_targs = {&ui_panels[CAPTURE_STATE][0]->x, &ui_panels[CAPTURE_STATE][1]->x, &ui_panels[CAPTURE_STATE][2]->x, &ui_panels[CAPTURE_STATE][3]->x};
            AnimationLink negative_tween_link = AnimationLink(&in_out_bezier, nullptr, negative_tween_targs, -1280.f, 1280.f);
            animation_links.push_back(negative_tween_link);
        }
    }

    animate();
    for(unsigned i = 0; i < ui_panels[SELECT_STATE].size(); i ++) {
        UI_Panel *panel = ui_panels[SELECT_STATE][i];
        panel->render();
    }
    if(exiting_stage_2) {
        for(unsigned i = 0; i < ui_panels[CAPTURE_STATE].size(); i ++) {
            UI_Panel *panel = ui_panels[CAPTURE_STATE][i];
            panel->render();
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
            float frame = 5.f - ((float)countdown);
            ((Animated_Panel *)panel)->set_frame(frame);
        }

        if(!(countdown == 0 && (i == 2 || i == 3))) {
            panel->render();
        }
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

void load_interface() {
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

void run_state(float dT) {
    animation_links = std::vector<AnimationLink>();

    if(state == IDLE_STATE) {
        do_idle_state(dT);
    }
    else if(state == SELECT_STATE) {
        do_select_state(dT);
    }
    else if(state == CAPTURE_STATE) {
        do_capture_state(dT);
    }

    unanimate();
}