/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
 *
 * CARMEN is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option)
 * any later version.
 *
 * CARMEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General 
 * Public License along with CARMEN; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

#include <carmen/carmen.h>
#include <ctype.h>
#include "map_io.h"
#include "map_graphics.h"

#include <wand/magick_wand.h>

void imgtocarmen_print_usage(char* argv0) {
   carmen_die("Usage: %s -resolution/-res/--res/--resolution/-r <map_resolution> <image_filename> <carmen_filename>\n", argv0);}

int
main(int argc, char **argv) 
{
  char *in_filename, *out_filename;
  carmen_FILE *out_fp;
  char buf[1024];
  carmen_map_p map;
  int i;
  double resolution;
  resolution = -1.0;

  for (i=1; i<argc-2; i++) {
    if ( ( (!strcmp(argv[i],"-resolution")) || 
	   (!strcmp(argv[i],"--resolution")) ||
	   (!strcmp(argv[i],"-res")) ||
	   (!strcmp(argv[i],"--res")) || 
	   (!strcmp(argv[i],"-r")) ) && 
	 (argc>i+1)) {
      resolution = atof(argv[++i]);
    }
    else {
      carmen_warn("Error: unkown parameter %s\n",argv[i]);
      imgtocarmen_print_usage(argv[0]);
    }
  }

  if (argc < 3  || resolution < 0.0)
    imgtocarmen_print_usage(argv[0]);
  
  gdk_init(&argc,&argv);
  
  in_filename  = argv[argc-2];
  out_filename = argv[argc-1];

  if (carmen_file_exists(out_filename))
    {
      printf("Overwrite %s? ", out_filename);
      scanf("%s", buf);
      if (tolower(buf[0]) != 'y')
	exit(0);
    }

  map = carmen_map_imagefile_to_map(in_filename, resolution);

  out_fp = carmen_fopen(out_filename, "w");
  if (out_fp == NULL)
    carmen_die("Couldn't open %s for writing : %s\n", out_filename, 
	       strerror(errno));

  if (carmen_map_write_id(out_fp) < 0)
    carmen_die_syserror("Couldn't write map id to %s", out_filename);

  sprintf(buf, "Created from %s", out_filename);
  if (carmen_map_write_creator_chunk(out_fp, "img_to_map", buf) < 0)
    carmen_die_syserror("Couldn't write creator chunk to %s", out_filename);

  if (carmen_map_write_gridmap_chunk(out_fp, map->map, map->config.x_size,
				     map->config.y_size, 
				     map->config.resolution) < 0)
    carmen_die_syserror("Couldn't write gridmap chunk to %s", out_filename);
  
  carmen_fclose(out_fp);

  return 0;
}

