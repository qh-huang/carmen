#ifndef CANON_S40_H
#define CANON_S40_H

#include "usb.h"

/* four different transfer modes, they can be combined as well */
#define    THUMB_TO_PC              0x0001
#define    FULL_TO_PC               0x0002
#define    THUMB_TO_DRIVE           0x0004
#define    FULL_TO_DRIVE            0x0008

/* open a USB connection the camera */
usb_dev_handle *canon_open_camera(void);

/* initialize USB connection to the camera */
int canon_initialize_camera(usb_dev_handle *camera_handle);

/* identify camera */
int canon_identify_camera(usb_dev_handle *camera_handle);

/* get camera capabilities */
int canon_get_picture_abilities(usb_dev_handle *camera_handle);

/* turn off the LCD display and lock the keys */
int canon_lock_keys(usb_dev_handle *camera_handle);

/* get the power supply status */
int canon_get_power_status(usb_dev_handle *camera_handle);

/* initialize remote camera control mode */
int canon_rcc_init(usb_dev_handle *camera_handle);

/* unknown remote camera control command */
int canon_rcc_unknown(usb_dev_handle *camera_handle, int transfer_mode);

/* get release parameters */
int canon_rcc_get_release_params(usb_dev_handle *camera_handle);

/* take a picture */
int canon_rcc_release_shutter(usb_dev_handle *camera_handle);

/* set the capture transfer mode */
int canon_rcc_set_transfer_mode(usb_dev_handle *camera_handle, 
				int transfer_mode);

/* download the captured thumbnail image */
int canon_rcc_download_thumbnail(usb_dev_handle *camera_handle,
				 unsigned char **thumbnail, 
				 int *thumbnail_length);

/* dowload the captured full image */
int canon_rcc_download_full_image(usb_dev_handle *camera_handle,
				  unsigned char **image,
				  int *image_length);

/* exit remote capture control mode */
int canon_rcc_exit(usb_dev_handle *camera_handle);

/* initialize remote capture control */
int canon_initialize_capture(usb_dev_handle *camera_handle, int transfer_mode);

/* capture image wrapper */
int canon_capture_image(usb_dev_handle *camera_handle, 
			unsigned char **thumbnail, int *thumbnail_length,
			unsigned char **image, int *image_length);

/* stop capturing */
int canon_stop_capture(usb_dev_handle *camera_handle);

/* close USB connection to camera */
void canon_close_camera(usb_dev_handle *camera_handle);

#endif
