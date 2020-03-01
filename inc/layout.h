#pragma once

#include <Magick++.h>
#include <string>

typedef struct {
    Magick::Geometry img_dimensions;
    bool landscape;
    unsigned img_count;
    Magick::Geometry initial_pos;
    unsigned border;
    std::string layout_template;
    bool mirror;
} layout_t;

const layout_t vertical_layout_a = {Magick::Geometry(530, 400), true, 3,
    Magick::Geometry(0, 0, 35, 130), 30, "../data/layouts/vertical_layout.png", true};

const layout_t vertical_layout_b = {Magick::Geometry(530, 400), true, 3,
    Magick::Geometry(0, 0, 35, 130), 30, "../data/layouts/vertical_layout.png", false};

const layout_t polaroid_layout_a = {Magick::Geometry(814, 714), false, 1,
    .initial_pos = Magick::Geometry(0, 0, 30, 125), 30, "../data/layouts/polaroid_layout.png", true};

const layout_t polaroid_layout_b = {Magick::Geometry(814, 714), false, 1,
    .initial_pos = Magick::Geometry(0, 0, 30, 125), 30, "../data/layouts/polaroid_layout.png", false};

const layout_t mini_polaroid_layout_a = {Magick::Geometry(540, 440), true, 2,
    Magick::Geometry(0, 0, 35, 130), 434, "../data/layouts/mini_polaroid.png", true};

const layout_t mini_polaroid_layout_b = {Magick::Geometry(540, 440), true, 2,
    Magick::Geometry(0, 0, 35, 130), 434, "../data/layouts/mini_polaroid.png", false};

const layout_t big_four_layout = {Magick::Geometry(540, 540), true, 2,
    Magick::Geometry(0, 0, 35, 130), 30, "../data/layouts/big_four_layout.png", false};

Magick::Image init_layout(layout_t *layout);

void process_capture(Magick::Image *layout_template, std::string img_path, std::string dst);
//bool captures_complete(void);

//void cleanup(void);
