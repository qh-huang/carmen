#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define      MAX_URBS           10000

#define      BULK_OUT_HAND      0x821f5e60
#define      BULK_IN_HAND       0x821f5e7c
#define      INT_IN_HAND        0x821438f8

#define      OUT                1
#define      IN                 2

#define      CONTROL            1
#define      BULK               2
#define      INT                3
#define      UNKNOWN            4
#define      IGNORED            5

typedef struct {
  int num_lines, max_lines;
  char **lines;
} urb_text_t, *urb_text_p;

typedef struct {
  urb_text_t in, out;

  int active;
  int urb_type;
  int direction;
  int length;
  int incomplete;
  int request, value, index;
  unsigned char *buffer;
  double timestamp;
} urb_t, *urb_p;

FILE *fp;

urb_t urb[MAX_URBS];
int highest_urb = 0;

/* cut off the \n and \r characters at the end of a line of text */

char *chomp(char *str)
{
  while(strlen(str) > 0 && 
	(str[strlen(str) - 1] == '\n' || str[strlen(str) - 1] == '\r'))
    str[strlen(str) - 1] = '\0';
}

/* advance to the next word from a string */

char *next_word(char *str)
{
  char *mark = str;

  if(str == NULL)
    return NULL;
  while(mark[0] != '\0' && !isblank(mark[0]))
    mark++;
  if(mark[0] == '\0')
    return NULL;
  while(mark[0] != '\0' && isblank(mark[0]))
    mark++;
  if(mark[0] == '\0')
    return NULL;
  return mark;
}

/* return a pointer to the "nth" word of a string */

char *nth_word(char *str, int n)
{
  char *mark;
  int i;

  mark = str;
  for(i = 0; i < n - 1; i++)
    mark = next_word(mark);
  return mark;
}

void clear_urb_text(urb_text_p text)
{
  text->num_lines = 0;
  text->max_lines = 10;
  text->lines = (char **)calloc(sizeof(char *), text->max_lines);  
}

void clear_urb_list(urb_p urb)
{
  int i;

  for(i = 0; i < MAX_URBS; i++) {
    clear_urb_text(&(urb[i].in));
    clear_urb_text(&(urb[i].out));

    urb[i].active = 0;
    urb[i].buffer = NULL;
    urb[i].request = 0;
    urb[i].value = 0;
    urb[i].index = 0;
    urb[i].incomplete = 0;
  }
}

void add_line_to_urb(urb_text_p urb, char *line)
{
  if(urb->num_lines == urb->max_lines) {
    urb->max_lines += 10;
    urb->lines = realloc(urb->lines, sizeof(char *) * urb->max_lines);
  }
  urb->lines[urb->num_lines] = (char *)malloc(strlen(line) + 1);
  strcpy(urb->lines[urb->num_lines], line);
  (urb->num_lines)++;
}

void cut_logfile_into_urbs(FILE *fp, urb_p urb)
{
  char line[2000], *mark;
  long int line_count;
  double line_timestamp;
  int current_urb = -1;
  int i, out_urb_count = 0, in_urb_count = 0;
  int direction = OUT;

  while(!feof(fp)) {
    if(fgets(line, 2000, fp)) {
      chomp(line);
      sscanf(line, "%ld %lf", &line_count, &line_timestamp);
      mark = next_word(next_word(line));
      
      if(mark != NULL) {
	if(strncmp(mark, ">>>>", 4) == 0) {
	  sscanf(mark, ">>>>>>> URB %d", &current_urb);
	  direction = OUT;
	}
	else if(strncmp(mark, "<<<<", 4) == 0) {
	  sscanf(mark, "<<<<<<< URB %d", &current_urb);
	  direction = IN;
	}
	if(current_urb >= 0) {
	  if(direction == OUT)
	    add_line_to_urb(&(urb[current_urb].out), line);
	  else 
	    add_line_to_urb(&(urb[current_urb].in), line);
	}
      }
    }
  }
  
  for(i = 0; i < MAX_URBS; i++) {
    if(urb[i].out.num_lines > 0) {
      out_urb_count++;
      highest_urb = i;
    }
    if(urb[i].in.num_lines > 0) {
      in_urb_count++;
      highest_urb = i;
    }
  }
  fprintf(stderr, "Read %d outbound URBs\n", out_urb_count);
  fprintf(stderr, "Read %d inbound URBs\n", in_urb_count);
}

void process_urbs(void)
{
  int i, j;
  char dir[1000];
  int temp, current_line, count, hand;

  for(i = 1; i < MAX_URBS; i++) {
    if(urb[i].out.num_lines > 0 && urb[i].in.num_lines > 0) {
      urb[i].active = 1;
      
      /* get timestamp */
      sscanf(nth_word(urb[i].out.lines[0], 2), "%lf", 
	     &urb[i].timestamp);
      
      /* process control messages */
      if(strncmp(nth_word(urb[i].in.lines[1], 3),
		 "-- URB_FUNCTION_CONTROL", 23) == 0 &&
	 strncmp(nth_word(urb[i].out.lines[1], 3),
		 "-- URB_FUNCTION_VENDOR", 22) == 0) {
	urb[i].urb_type = CONTROL;
	
	/* get message direction */
	sscanf(nth_word(urb[i].in.lines[3], 6) + 1, "%s", dir);
	if(strncmp(dir, "USBD_TRANSFER_DIRECTION_IN", 26) == 0)
	  urb[i].direction = IN;
	else if(strncmp(dir, "USBD_TRANSFER_DIRECTION_OUT", 27) == 0)
	  urb[i].direction = OUT;
	
	/* get message length */
	sscanf(nth_word(urb[i].in.lines[4], 5), "%x", 
	       &(urb[i].length));
	
	/* get message contents */
	count = 0;
	urb[i].buffer = (char *)calloc(1, urb[i].length);
	if(urb[i].direction == IN) {
	  current_line = 8;
	  while(count < urb[i].length) {
	    sscanf(nth_word(urb[i].in.lines[current_line], 3), "%x",
		   &temp);
	    urb[i].buffer[count] = (unsigned char)temp;
	    count++;
	    current_line++;
	    if(current_line >= urb[i].in.num_lines) {
	      urb[i].incomplete = 1;
	      break;
	    }
	    if(count > 0 && count % 16 == 0)
	      current_line++;
	  }
	  
	  /* get request, value, & index */
	  sscanf(nth_word(urb[i].out.lines[8], 5), "%x",
		 &urb[i].request);
	  sscanf(nth_word(urb[i].out.lines[9], 5), "%x",
		 &urb[i].value);
	  sscanf(nth_word(urb[i].out.lines[10], 5), "%x",
		 &urb[i].index);	  
	}
	else if(urb[i].direction == OUT) {
	  current_line = 7;
	  while(count < urb[i].length) {
	    if(current_line >= urb[i].out.num_lines) {
	      urb[i].incomplete = 1;
	      break;
	    }
	    sscanf(nth_word(urb[i].out.lines[current_line], 3), "%x",
	       &temp);
	    urb[i].buffer[count] = (unsigned char)temp;
	    count++;
	    current_line++;
	    if(count > 0 && count % 16 == 0 && count != urb[i].length)
	      current_line++;
	  }
	  current_line += 2;
	  
      /* get request, value, & index */
	  if(!urb[i].incomplete) {
	    sscanf(nth_word(urb[i].out.lines[current_line], 5), "%x",
		   &urb[i].request);
	    sscanf(nth_word(urb[i].out.lines[current_line + 1], 5), "%x",
		   &urb[i].value);
	    sscanf(nth_word(urb[i].out.lines[current_line + 2], 5), "%x",
		   &urb[i].index);
	  }
	}
	
	printf("URB %d : %.3f : CONTROL %s (%d/0x%x bytes) %02x %04x %04x %s\n", 
	       i, urb[i].timestamp, (urb[i].direction == IN) ? "IN" : "OUT", 
	       urb[i].length, urb[i].length,
	       urb[i].request, urb[i].value, urb[i].index,
	       urb[i].incomplete ? "- INCOMPLETE" : "");
	if(urb[i].buffer != NULL)
	  for(j = 0; j < urb[i].length; j++) {
	    printf("%02x ", urb[i].buffer[j]);
	    if((j + 1) % 16 == 0 && j + 1 != urb[i].length)
	      printf("\n");
	  }
	printf("\n\n");
      }
      /* process bulk and interrupt messages */
      else if(strncmp(nth_word(urb[i].in.lines[1], 4),
		      "URB_FUNCTION_BULK", 17) == 0) {
	
	sscanf(nth_word(urb[i].in.lines[2], 5), "%x", &hand);
	if(hand == BULK_IN_HAND) {
	  urb[i].urb_type = BULK;
	  urb[i].direction = IN;
	  /* get message length */
	  sscanf(nth_word(urb[i].in.lines[4], 5), "%x", 
		 &(urb[i].length));
	  
	  printf("URB %d : %.3f : BULK IN %d/0x%x\n\n",
		 i, urb[i].timestamp, urb[i].length, urb[i].length);
	  
	}
	else if(hand == BULK_OUT_HAND) {
	  urb[i].urb_type = BULK;
	  urb[i].direction = OUT;
	  
	  printf("URB %d : %.3f : BULK OUT ???\n\n", i, urb[i].timestamp);
	}
	else if(hand == INT_IN_HAND) {
	  urb[i].urb_type = INT;
	  urb[i].direction = IN;
	  
	  /* get message length */
	  sscanf(nth_word(urb[i].in.lines[4], 5), "%x", 
		 &(urb[i].length));
	  
	  if(urb[i].length != 0)
	    printf("URB %d : %.3f : INT IN %d/0x%x\n\n", i, 
		   urb[i].timestamp, urb[i].length, urb[i].length);
	}
	
	
	
      }
      else if(strncmp(nth_word(urb[i].in.lines[1], 4),
		      "URB_FUNCTION_CONTROL", 20) == 0 ||
	      strncmp(nth_word(urb[i].in.lines[1], 4),
		      "URB_FUNCTION_SELECT", 19) == 0) {
	urb[i].urb_type = IGNORED;
	printf("URB %d : IGNORED CONTROL TRANSFER\n\n", i);
      }
      else {
	urb[i].urb_type = UNKNOWN;
	printf("URB %d : UNKNOWN TYPE\n\n", i);
      }
    }
    else if(urb[i].out.num_lines == 0 && urb[i].in.num_lines == 0) {
      printf("URB %d : MISSING URB\n\n", i);
    }
    else {
      printf("URB %d : HALF MISSING URB\n\n", i);
      urb[i].incomplete = 1;
    }
  }
}

void print_processed_urbs(urb_p urb)
{
  int i;

  for(i = 0; i <= highest_urb; i++)
    if(urb[i].active) {
      printf("URB %d : ", i);
      switch(urb[i].urb_type) {
      case IGNORED:
	printf("IGNORED CONTROL TRANSFER\n\n");
	break;
      case CONTROL:
	printf("CONTROL TRANSFER\n\n");
	break;
      case BULK:
	printf("BULK TRANSFER\n\n");
	break;
      case INT:
	printf("INT TRANSFER\n\n");
	break;
      }
    }
    else if(urb[i].incomplete)
      printf("URB %d : INCOMPLETE URB\n\n", i);
}

int main(int argc, char **argv)
{
  if(argc < 2) {
    fprintf(stderr, "Error: not enough arguments.\n");
    fprintf(stderr, "Usage: %s filename.\n", argv[0]);
    exit(1);
  }
  fp = fopen(argv[1], "r");
  if(fp == NULL) {
    fprintf(stderr, "Error: could not open file %s for reading.\n", argv[1]);
    exit(1);
  }

  clear_urb_list(urb);
  cut_logfile_into_urbs(fp, urb);
  process_urbs();
  print_processed_urbs(urb);
  return 0;
}
