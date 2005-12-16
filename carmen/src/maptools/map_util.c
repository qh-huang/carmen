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

/* An implementation of bicubic image interpolation, shamelessly borrowed
   from an implementation by Blake Carlson (blake-carlson@uiowa.edu). 
*/

static const float one_sixth = 1.0 / 6.0;

static float 
cubic_bspline(float x) 
{  
  float a, b, c, d;

  // Implement Eq 4.3-5 in four parts
  
  if((x + 2.0) <= 0.0) {
    a = 0.0;
  }
  else {
    a = pow((x + 2.0), 3.0);
  }

  if((x + 1.0) <= 0.0) {
    b = 0.0;
  }
  else {
    b = pow((x + 1.0), 3.0);
  }    

  if(x <= 0) {
    c = 0.0;
  }
  else {
    c = pow(x, 3.0);
  }  

  if((x - 1.0) <= 0.0) {
    d = 0.0;
  }
  else {
    d = pow((x - 1.0), 3.0);
  }
  
  return (one_sixth * (a - (4.0 * b) + (6.0 * c) - (4.0 * d)));
}

void
carmen_map_util_change_resolution(carmen_map_p map, double new_resolution)
{
  float *new_complete_map;
  float **new_map;
  carmen_map_config_t new_config;

  float **padded_map;
  float *padded_backing;

  int padded_x_size, padded_y_size;
  float scale_factor;

  int x, y;
  float f_x, f_y, a, b, tmp, r1, r2;
  int i_x, i_y, m, n;

  new_config.x_size = map->config.resolution/new_resolution * 
    map->config.x_size;
  new_config.y_size = map->config.resolution/new_resolution * 
    map->config.y_size;
  new_config.resolution = new_resolution;
  new_config.map_name = map->config.map_name;

  new_complete_map = (float *)
    calloc(new_config.x_size*new_config.y_size, sizeof(float));
  carmen_test_alloc(new_complete_map);

  new_map = (float **)calloc(new_config.x_size, sizeof(float *));
  carmen_test_alloc(new_map);
  for (x = 0; x < new_config.x_size; x++)
    new_map[x] = new_complete_map+x*new_config.y_size;

  padded_x_size = map->config.x_size+4;
  padded_y_size = map->config.y_size+4;

  padded_backing = (float *)calloc(padded_x_size*padded_y_size, sizeof(float));
  carmen_test_alloc(padded_backing);

  padded_map = (float **)calloc(padded_x_size, sizeof(float *));
  carmen_test_alloc(padded_map);
  for (x = 0; x < padded_x_size; x++)
    padded_map[x] = padded_backing+x*padded_y_size;

  for (x = 2; x < padded_x_size-2; x++) {
    memcpy(padded_map[x]+2, map->map[x-2], map->config.y_size*sizeof(float));
    memcpy(padded_map[x], padded_map[x]+2, 2*sizeof(float));
    memcpy(padded_map[x]+2+map->config.y_size, 
	   padded_map[x]+map->config.y_size, 
	   2*sizeof(float));
  }

  memcpy(padded_map[0], padded_map[2], padded_y_size*sizeof(float));
  memcpy(padded_map[1], padded_map[2], padded_y_size*sizeof(float));

  memcpy(padded_map[padded_x_size-2], padded_map[padded_x_size-3], 
	 padded_y_size*sizeof(float));
  memcpy(padded_map[padded_x_size-1], padded_map[padded_x_size-3], 
	 padded_y_size*sizeof(float));

  scale_factor = map->config.resolution / new_resolution;

  for (y = 0; y < new_config.y_size; y++) {
    f_y = y / scale_factor;
    i_y = carmen_trunc(f_y);
    a   = f_y - carmen_trunc(f_y);		
    for (x = 0; x < new_config.x_size; x++) {
      f_x = x / scale_factor;
      i_x = carmen_trunc(f_x);
      b   = f_x - carmen_trunc(f_x);
        
      // Implement EQ 14.5-3 here
      tmp = 0.0;
      for(m = -1; m < 3; m++) {
	r1 = cubic_bspline((float) m - a);
          
	for(n = -1; n < 3; n++) {
	  r2 = cubic_bspline(-1.0*((float)n - b)); 
	  tmp += padded_map[i_x+n+2][i_y+m+2] * r1 * r2;
	}
      }

      if (tmp < 0)
	tmp = -1;
      if (tmp > 1.0)
	tmp = 1.0;
      new_map[x][y] = tmp;
    }		
  }

  free(padded_map);
  free(padded_backing);

  free(map->map);
  free(map->complete_map);

  map->map = new_map;
  map->complete_map = new_complete_map;
  map->config = new_config;
}


void
carmen_map_minimize_gridmap(carmen_map_t *map, int *x_offset, int *y_offset)
{
  int x, y;
  
  int min_x = map->config.x_size, min_y = map->config.y_size;
  int max_x = 0, max_y = 0;

  int new_width, new_height;
  float **new_map;
  float *new_complete_map;

  for (x = 0; x < map->config.x_size; x++)
    for (y = 0; y < map->config.y_size; y++) 
      {
	if (map->map[x][y] != -1) 
	  {
	    min_x = carmen_fmin(min_x, x);
	    min_y = carmen_fmin(min_y, y);
	    max_x = carmen_fmax(max_x, x);
	    max_y = carmen_fmax(max_y, y);
	  }
      }

  new_width = max_x - min_x + 1;
  new_height = max_y - min_y + 1;

  new_complete_map = (float *)calloc(new_width*new_height, sizeof(float));
  carmen_test_alloc(new_complete_map);

  new_map = (float **)calloc(new_width, sizeof(float));
  carmen_test_alloc(new_map);

  for (x = 0; x < new_width; x++)
    {
      new_map[x] = new_complete_map+x*new_height;
      for (y = 0; y < new_height; y++) 
	memcpy(new_map[x], map->map[x+min_x]+min_y, new_height*sizeof(float));
    }

  free(map->map);
  map->map = new_map;

  free(map->complete_map);
  map->complete_map = new_complete_map;

  map->config.x_size = new_width;
  map->config.y_size = new_height;

  *x_offset = min_x;
  *y_offset = min_y; 
}

void
carmen_map_move_offlimits_chunk(carmen_offlimits_list_t *offlimits_list, 
				int x_offset, int y_offset)
{
  int i;

  for (i = 0; i < offlimits_list->list_length; i++) 
    {
      if (offlimits_list->offlimits[i].type == CARMEN_OFFLIMITS_POINT_ID) 
	{
	  offlimits_list->offlimits[i].x1 -= x_offset;
	  offlimits_list->offlimits[i].y1 -= y_offset;
	}
      else if (offlimits_list->offlimits[i].type == CARMEN_OFFLIMITS_LINE_ID ||
	       offlimits_list->offlimits[i].type == CARMEN_OFFLIMITS_RECT_ID)
	{
	  offlimits_list->offlimits[i].x1 -= x_offset;
	  offlimits_list->offlimits[i].y1 -= y_offset;
	  offlimits_list->offlimits[i].x2 -= x_offset;
	  offlimits_list->offlimits[i].y2 -= y_offset;
	}
    }
}


void
carmen_map_free_hmap(carmen_hmap_p hmap)
{
  int i;

  if (hmap->zone_names) {
    for (i = 0; i < hmap->num_zones; i++)
      free(hmap->zone_names[i]);
    free(hmap->zone_names);
  }
  if (hmap->links) {
    for (i = 0; i < hmap->num_links; i++) {
      free(hmap->links[i].keys);
      free(hmap->links[i].points);
    }
    free(hmap->links);
  }
}

void carmen_rotate_places_chunk(carmen_place_p places, int num_places, 
				carmen_map_p map, int rotation)
{
  int index;
  int width, height;
  double tmp;

  width = map->config.x_size;
  height = map->config.y_size;

  for (index = 0; index < num_places; index++) {
    if (rotation == 1) {
      tmp = places[index].x;
      places[index].x = height-places[index].y;
      places[index].y = tmp;
    } else if (rotation == 2) {
      places[index].x = width - places[index].x;
      places[index].y = height - places[index].y;
    } else if (rotation == 3) {
      tmp = places[index].x;
      places[index].x = places[index].y;
      places[index].y = width-tmp;
    }
  }
}

void carmen_rotate_gridmap(carmen_map_p map, int rotation) 
{
  int index, x = 0, y = 0;
  int height, width;
  float *new_map, *new_map_ptr;
  
  new_map = (float *)calloc(map->config.x_size*map->config.y_size, 
			    sizeof(float));
  carmen_test_alloc(new_map);
  
  new_map_ptr = new_map;
  width = map->config.x_size;
  height = map->config.y_size;
  for (index = 0; index < width*height; index++) {      
    if (rotation == 0) {
      x = index / height;
      y = index % height;
    } else if (rotation == 1) {
      x = width - (index % width) - 1;
      y = index / width;
    } else if (rotation == 2) {
      x = width - (index / height) - 1;
      y = height - (index % height) - 1;
    } else if (rotation == 3) {
      x = index % width;
      y = width - index / width - 1;
    }
    *(new_map_ptr++) = map->map[x][y];
  }

  free(map->complete_map);
  map->complete_map = new_map;

  free(map->map);
  
  if (rotation == 1 || rotation == 3) {
    height = map->config.x_size;
    width = map->config.y_size;
    map->config.x_size = width;
    map->config.y_size = height;
  }

  map->map = (float **)calloc(map->config.x_size, sizeof(float *));
  carmen_test_alloc(map->map);
  
  for (index = 0; index < map->config.x_size; index++) 
    map->map[index] = map->complete_map+index*map->config.y_size;
}

