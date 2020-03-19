#pragma once

bool connect_to_camera(void);
void disconnect_camera(void);
bool take_preview(char **data_ptr, unsigned long *data_size);
//bool take_preview(void);
bool take_full_image(unsigned capture_number);
