#include <carmen/carmen.h>
#include "jpeglib.h"

METHODDEF(void) init_source(j_decompress_ptr cinfo
			    __attribute__ ((unused)))
{
}

METHODDEF(void) term_source(j_decompress_ptr cinfo
			    __attribute__ ((unused)))
{
}

METHODDEF(boolean) fill_input_buffer(j_decompress_ptr cinfo
				     __attribute__ ((unused)))
{
  return TRUE;
}

METHODDEF(void) skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  cinfo->src->next_input_byte += (size_t)num_bytes;
  cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
}

GLOBAL(void) jpeg_mem_src(j_decompress_ptr cinfo, unsigned char *buf, 
			  int length)
{
  if(cinfo->src == NULL)
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small)((j_common_ptr)cinfo,
				 JPOOL_PERMANENT,
				 sizeof(struct jpeg_source_mgr));
  
  cinfo->src->next_input_byte = buf;
  cinfo->src->bytes_in_buffer = length;
  cinfo->src->init_source = init_source;
  cinfo->src->fill_input_buffer = fill_input_buffer;
  cinfo->src->skip_input_data = skip_input_data;
  cinfo->src->resync_to_restart = jpeg_resync_to_restart;
  cinfo->src->term_source = term_source;
}

void read_jpeg_from_memory(unsigned char *image_in, int image_length,
			   unsigned char **image_out, int *image_cols,
			   int *image_rows)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr err_mgr;
  JSAMPARRAY buffer;
  int row_stride;

  cinfo.err = jpeg_std_error(&err_mgr);
  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, image_in, image_length);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  *image_rows = cinfo.output_height;   *image_cols = cinfo.output_width;
  *image_out = (unsigned char *)calloc(*image_rows * (*image_cols), 
				       cinfo.output_components);
  carmen_test_alloc(*image_out);

  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, 
				      row_stride, 1);
  while(cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    memcpy(*image_out + (cinfo.output_scanline - 1) * row_stride, buffer[0], 
	   row_stride);
  }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
}
