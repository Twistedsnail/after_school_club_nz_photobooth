#include "layout.h"
#include <vector>
#include <cstring>

static layout_t *layout_ptr = nullptr;
static std::vector<Magick::Image> captures;
static unsigned capture_number = 0;

#define LAYOUT_WIDTH    1181
#define LAYOUT_HEIGHT   1748

Magick::Image init_layout(layout_t *layout) {
    layout_ptr = layout;
    Magick::Image layout_template = Magick::Image(layout_ptr->layout_template);
    captures = std::vector<Magick::Image>();
    capture_number = 0;
    return layout_template;
}

void process_capture(Magick::Image *layout_template, std::string img_path, std::string dst) {
    Magick::Image capture(img_path);
    printf("\t-opened\n");

    float original_ratio = (float)capture.columns() / (float)capture.rows();
    float new_ratio = (float)layout_ptr->img_dimensions.width() / (float)layout_ptr->img_dimensions.height();

    printf("Ratio %.3f to %.3f\n", original_ratio, new_ratio);

    if(original_ratio > new_ratio) {
        unsigned new_width = (unsigned)(new_ratio * (float)capture.rows());
        unsigned offset = (capture.columns() - new_width) / 2;
        printf("Cropping to %i, %i, %i, %i\n", offset, 0, new_width, capture.rows());
        capture.crop(Magick::Geometry(new_width, capture.rows(), offset));
    }
    else if(original_ratio < new_ratio) {
        unsigned new_height = (unsigned)((float)capture.columns() / new_ratio);
        unsigned offset = (capture.rows() - new_height) / 2;
        printf("Cropping to %i, %i, %i, %i\n", 0, offset, capture.columns(), new_height);
        capture.crop(Magick::Geometry(capture.columns(), new_height, 0, offset));
    }
    else printf("No crop needed\n");

    capture.resize(layout_ptr->img_dimensions);
    if(!layout_ptr->landscape) capture.rotate(90.0);
    capture.syncPixels();
    printf("\t-edited\n");

    Magick::Geometry capture_area;
    unsigned pos_number = capture_number;
    if(!layout_ptr->mirror && capture_number >= layout_ptr->img_count) pos_number -= layout_ptr->img_count;

    if(layout_ptr->landscape) {
        capture_area = Magick::Geometry(layout_ptr->img_dimensions.width(), layout_ptr->img_dimensions.height(),
                                        layout_ptr->initial_pos.xOff(),
                                        layout_ptr->initial_pos.yOff() +
                                        layout_ptr->img_dimensions.height() * pos_number +
                                        layout_ptr->border * pos_number);
    }
    else {
        capture_area = Magick::Geometry(layout_ptr->img_dimensions.height(), layout_ptr->img_dimensions.width(),
                                        LAYOUT_WIDTH - (layout_ptr->initial_pos.yOff() +
                                        layout_ptr->img_dimensions.height() * (pos_number + 1) +
                                        layout_ptr->border * pos_number),
                                        layout_ptr->initial_pos.xOff());
    }

    const MagickLib::PixelPacket *src_pixels = capture.getConstPixels(0, 0, capture_area.width(), capture_area.height());

    MagickLib::PixelPacket *dst_pixels;
    if(capture_number < layout_ptr->img_count) {
        dst_pixels = layout_template->setPixels(capture_area.xOff(), capture_area.yOff(), capture_area.width(), capture_area.height());
        memcpy(dst_pixels, src_pixels, sizeof(MagickLib::PixelPacket) * layout_ptr->img_dimensions.width() * layout_ptr->img_dimensions.height());
        layout_template->syncPixels();
    }

    if(layout_ptr->mirror || (!layout_ptr->mirror && capture_number >= layout_ptr->img_count)) {
        if(layout_ptr->landscape) {
            dst_pixels = layout_template->setPixels(LAYOUT_WIDTH - capture_area.width() - capture_area.xOff(), capture_area.yOff(), capture_area.width(), capture_area.height());
        }
        else {
            dst_pixels = layout_template->setPixels(capture_area.xOff(), LAYOUT_HEIGHT - capture_area.yOff()- capture_area.height(), capture_area.width(), capture_area.height());
        }
        memcpy(dst_pixels, src_pixels, sizeof(MagickLib::PixelPacket) * layout_ptr->img_dimensions.width() * layout_ptr->img_dimensions.height());
        layout_template->syncPixels();
    }

    capture_number ++;
    if((layout_ptr->mirror && capture_number == layout_ptr->img_count) || (!layout_ptr->mirror && capture_number == 2 * layout_ptr->img_count)) layout_template->write(dst);
}
