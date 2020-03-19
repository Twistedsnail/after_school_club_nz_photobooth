#include "camera_manager.h"
#include "display_manager.h"
#include "layout.h"
#include "curl_manager.h"
#include "statemachine.h"

#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <thread>

bool main_ready = true;
bool preview_taken = false;

static char *preview_blob_data = nullptr;
static unsigned long preview_blob_size = 0;

void camera_thread() {
    while (should_run()) {
        if((state == IDLE_STATE || state == CAPTURE_STATE) && main_ready && ! preview_taken) {
            if(take_preview(&preview_blob_data, &preview_blob_size)) {
                main_ready = false;
                preview_taken = true;
            }
        }
    }

    printf("Exited camera thread\n");
}

int main() {
    Magick::InitializeMagick("");
	srand(time(NULL));

	//curl_test();

    if(!connect_to_camera()) return 0;
    open_window();

    /*Magick::Image layout_template = init_layout(&polaroid_layout_b);

    const unsigned n = 2;
    const char *paths[n] = {"/media/pi/photobooth/trulove/received_346439095950940.jpeg", "/media/pi/photobooth/trulove/received_292946184635815.jpeg"};

    for(unsigned i = 0; i < n; i++) {
        printf("Processing test_img %i\n", i);
        process_capture(&layout_template, paths[i], "/media/pi/photobooth/polaroid1.png");
    }
    printf("Layout complete\n");*/

    std::thread cam_thread = std::thread(camera_thread);

    while(should_run()) {
        if(! main_ready && preview_taken) {
            preview_taken = false;
            if(state == IDLE_STATE) {
                load_blurred_texture(preview_blob_data, &preview_blob_size);
            }
            else if(state == CAPTURE_STATE) {
                load_preview(preview_blob_data, &preview_blob_size);
            }
            main_ready = true;
        }
        update_window();
        //if(get_triggered()) {
            //if(take_full_image(capture_number++)) {
                //show_full = true;
                //printf("Loading full\n");
                //load_full_texture("../data/capture.cr2");
                //printf("Loaded!\n");
            //}
        //}
    }

    disconnect_camera();

    return 0;
}
