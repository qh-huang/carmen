#ifndef CANON_INTERFACE_H
#define CANON_INTERFACE_H

#include "canon_messages.h"

carmen_canon_image_message *carmen_canon_get_image(int thumbnail_over_ipc,
						   int image_over_ipc,
						   int image_to_disk);

void carmen_canon_free_image(carmen_canon_image_message **message);

#endif
