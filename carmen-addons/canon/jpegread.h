#ifndef JPEGREAD_H
#define JPEGREAD_H

void read_jpeg_from_memory(unsigned char *image_in, int image_length,
			   unsigned char **image_out, int *image_cols,
			   int *image_rows);

#endif
