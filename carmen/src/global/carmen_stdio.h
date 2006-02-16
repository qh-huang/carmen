
/** @addtogroup global libglobal **/
// @{

/** \file carmen_stdio.h
 * \brief stdio-functions for CARMEN in libglobal. This library supports gzipped files.
 *
 * stdio-functions for CARMEN. Support reading and writing gzipped files.
 **/

#ifndef DGC_MY_STDIO_H
#define DGC_MY_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <carmen/carmen.h>
#ifndef NO_ZLIB
#include <zlib.h>
#endif

typedef struct {
  int compressed;
  FILE *fp;
#ifndef NO_ZLIB
  gzFile *comp_fp;
#endif
} carmen_FILE;

carmen_FILE *carmen_fopen(const char *filename, const char *mode);

int carmen_fgetc(carmen_FILE *fp);

int carmen_feof(carmen_FILE *fp);

int carmen_fseek(carmen_FILE *fp, long offset, int whence);

long carmen_ftell(carmen_FILE *fp);

int carmen_fclose(carmen_FILE *fp);

size_t carmen_fread(void *ptr, size_t size, size_t nmemb, carmen_FILE *fp);

size_t carmen_fwrite(const void *ptr, size_t size, size_t nmemb, 
		     carmen_FILE *fp);

char *carmen_fgets(char *s, int size, carmen_FILE *fp);

int carmen_fputc(int c, carmen_FILE *fp);

void carmen_fprintf(carmen_FILE *fp, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
// @}
