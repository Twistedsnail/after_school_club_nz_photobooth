#include "camera_manager.h"
#include <stdio.h>
#include <gphoto2/gphoto2.h>
#include <string>
#include <cstring>

static Camera *camera;
static GPContext *context;

static const char *get_camera_status_string(int status) {
    switch(status) {
        case GP_OK: return "OK";
        case GP_ERROR: return "ERROR";
        case GP_ERROR_BAD_PARAMETERS: return "BAD_PARAMETERS";
        case GP_ERROR_NO_MEMORY: return "OUT_OF_MEMORY";
        case GP_ERROR_LIBRARY: return "DRIVER_ERROR";
        case GP_ERROR_UNKNOWN_PORT: return "UNKNOWN_PORT";
        case GP_ERROR_NOT_SUPPORTED: return "NOT_SUPPORTED";
        case GP_ERROR_IO: return "IO_ERROR";
        case GP_ERROR_FIXED_LIMIT_EXCEEDED: return "INTERNAL_BUFFER_OVERFLOW";
        case GP_ERROR_TIMEOUT: return "TIMEOUT";
        case GP_ERROR_IO_SUPPORTED_SERIAL: return "SERIAL_IO_UNSUPPORTED";
        case GP_ERROR_IO_SUPPORTED_USB: return "USB_UNSUPPORTED";
        case GP_ERROR_IO_INIT: return "IO_INIT_FAILED";
        case GP_ERROR_IO_READ: return "READ_FAILED";
        case GP_ERROR_IO_WRITE: return "WRITE_FAILED";
        case GP_ERROR_IO_UPDATE: return "SETTINGS_UPDATE_FAILED";
        case GP_ERROR_IO_SERIAL_SPEED: return "SPEED_UNSUPPORTED";
        case GP_ERROR_IO_USB_CLEAR_HALT: return "USB_CLEAR_HALT_FAILED";
        case GP_ERROR_IO_USB_FIND: return "COULDNT_FIND_USB";
        case GP_ERROR_IO_USB_CLAIM: return "COULDNT_CLAIM_USB";
        case GP_ERROR_IO_LOCK: return "COULDNT_LOCK_USB";
        case GP_ERROR_HAL: return "UNSPECIFIED_HAL_ERROR";
        default: return "UNKNOWN_ERROR";
    }
}

static void context_error(GPContext *error_context, const char *str, void *data) {
    printf("Context Error: %s\n", str);
}

static void log_error(GPLogLevel level, const char *domain, const char *str, void *data) {
    printf("GPLog: %i in %s -> %s\n", level, domain, str);
}

/*static void poll_camera() {
    CameraEventType event_type;
    void *event_data;
    int status; = gp_camera_wait_for_event(camera, 1, &event_type, &event_data, context);
    if(status == GP_OK) {

    }
}*/

void connect_to_camera() {
    context = gp_context_new();
    gp_context_set_error_func(context, context_error, NULL);
    gp_log_add_func(GP_LOG_ERROR, log_error, NULL);

    gp_camera_new(&camera);

    for(unsigned attempts = 0; attempts < 3; attempts++) {
        int status = gp_camera_init(camera, context);
        if(status != GP_OK) printf("Camera connection attempt %i failed\n", attempts + 1);
        else {
            printf("Successful camera connection:\n");
            break;
        }
    }

    CameraText info_text;
    gp_camera_get_summary(camera, &info_text, context);
    printf("%s\n", info_text.text);
}

void disconnect_camera() {
    gp_camera_exit(camera, context);
}

#define CAM_FUNC_CHECK(cmd, msg)    status = cmd;\
                                    if(status != GP_OK) {\
                                        printf("%s: %s\n", msg, get_camera_status_string(status));\
                                        return false;\
                                    }

/*bool take_preview() {
    CameraFile *file;
    int status;

    CAM_FUNC_CHECK(gp_file_new(&file), "File creation error");
    CAM_FUNC_CHECK(gp_camera_capture_preview(camera, file, context), "Preview capture error");
    CAM_FUNC_CHECK(gp_file_save(file, "../data/preview.jpg"), "File save error");
    gp_file_unref(file);
    return true;
}*/

bool take_preview(char **data_ptr, unsigned long *data_size) {
    CameraFile *file;
    int status;

    char *temp_dat;

    CAM_FUNC_CHECK(gp_file_new(&file), "File creation error");
    CAM_FUNC_CHECK(gp_camera_capture_preview(camera, file, context), "Preview capture error");
    CAM_FUNC_CHECK(gp_file_get_data_and_size(file, (const char **)&temp_dat, data_size), "File get error");

    *data_ptr = (char *)malloc(*data_size);
    std::memcpy(*data_ptr, temp_dat, *data_size);
    //printf("File captured %lu\n", *data_size);

    gp_file_unref(file);
    return true;
}

bool take_full_image(unsigned capture_number) {
    CameraFile *file;
    CameraFilePath path;
    int status;
    std::string dst_path = "../data/capture" + std::to_string(capture_number) + ".cr2";

    CAM_FUNC_CHECK(gp_camera_capture(camera, GP_CAPTURE_IMAGE, &path, context), "Full capture error");
    CAM_FUNC_CHECK(gp_file_new(&file), "File creation error");
    CAM_FUNC_CHECK(gp_camera_file_get(camera, path.folder, path.name, GP_FILE_TYPE_NORMAL, file, context), "File retrieve error");
    CAM_FUNC_CHECK(gp_file_save(file, dst_path.c_str()), "File save error");
    gp_file_unref(file);
    return true;
}
