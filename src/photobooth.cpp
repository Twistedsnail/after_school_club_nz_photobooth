#include "camera_manager.h"
#include "display_manager.h"
#include "layout.h"
#include "curl_manager.h"

#include <stdlib.h>
#include <time.h>
#include <cstdlib>

static unsigned capture_number = 0;
static char *preview_blob_data = nullptr;
static unsigned long preview_blob_size = 0;

int main() {
    Magick::InitializeMagick("");
	srand(time(NULL));

	//curl_test();

    connect_to_camera();
    open_window();

    /*Magick::Image layout_template = init_layout(&polaroid_layout_b);

    const unsigned n = 2;
    const char *paths[n] = {"/media/pi/photobooth/trulove/received_346439095950940.jpeg", "/media/pi/photobooth/trulove/received_292946184635815.jpeg"};

    for(unsigned i = 0; i < n; i++) {
        printf("Processing test_img %i\n", i);
        process_capture(&layout_template, paths[i], "/media/pi/photobooth/polaroid1.png");
    }
    printf("Layout complete\n");*/

    while(should_run()) {
        //take_preview();
        /*if(take_preview(&preview_blob_data, &preview_blob_size)) {
            load_preview(preview_blob_data, &preview_blob_size);
        }*/
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
