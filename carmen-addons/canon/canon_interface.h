#ifndef CANON_INTERFACE_H
#define CANON_INTERFACE_H

#include "canon.h"
#include "canon_messages.h"
#include "jpegread.h"

carmen_canon_image_message *carmen_canon_get_image(int thumbnail_over_ipc,
						   int image_over_ipc,
						   int image_to_disk,
						   int flash_mode);

void carmen_canon_free_image(carmen_canon_image_message **message);

void
carmen_canon_subscribe_preview_message(carmen_canon_preview_message *preview,
				       carmen_handler_t handler,
				       carmen_subscribe_t subscribe_how);

int carmen_canon_start_preview_command(void);

int carmen_canon_stop_preview_command(void);

#endif
