#include <carmen/carmen.h>

/*
 * This program was written by Dirk Hähnel.
 *
 * Log files contained an incorrect parameter and sync output.
 * This program repairs the files by re-sorting and putting the
 * parameter and sync definitions at the front. This can be
 * useful for merging logfiles. 
 *
 */ 

void
print_usage( void )
{
  fprintf( stderr,
	   "usage: log_timestamp_repair <LOG-FILE>\n");
}

#define REF_ALLOC_STEP   200

typedef struct {
  double    time;
  int       type;
  int       posptr;
} line_ref;


#define MAX_LINE_LENGTH  65536

int
lineCompare( const void *a, const void *b )
{
  static line_ref v1, v2;
  v1 = *(const line_ref *)a; 
  v2 = *(const line_ref *)b;
  if (v1.time==v2.time) {
    return(v1.type-v2.type);
  }
  return(v1.time>v2.time);
}

#define MAX_LINE_ITEMS  30000

double
getTimeStamp( char * line )
{
    static char copy[MAX_LINE_LENGTH];
    int  i = 0;
    char * ptr[MAX_LINE_ITEMS];
    double val;

    strncpy( copy, line, MAX_LINE_LENGTH );
    ptr[i] = strtok( copy, " ");
    do {
	i++;
	ptr[i] = strtok( NULL, " ");
    } while ( ptr[i] != NULL && i<MAX_LINE_ITEMS );
    i--;
    
    if (i>2 && i!=MAX_LINE_ITEMS-2) {
	val = atof(ptr[i-2]);
    } else {
	val = 0.0;
    }
    return(val);
}

int
main(int argc, char *argv[])
{
  FILE           * fp;
  long             filesize = 0;
  unsigned char  * f;
  char           * s;
  char             b[MAX_LINE_LENGTH];
  char             cmd[MAX_LINE_LENGTH];
  line_ref       * l;
  int              fptr  = 0;
  int              lines = 0;
  int              fend  = 0;
  int              i, len;

  if (argc!=2) {
    print_usage();
    return(1);
  }

  if ((fp = fopen( argv[1], "r")) == 0){
    fprintf(stderr, "ERROR: can't open file %s\n", argv[1] );
    exit(1);
  }

  do{
    if ((s = fgets(b,MAX_LINE_LENGTH,fp)) == NULL) {
      fend=1;
    } else {
      lines++;
    }
  } while (!fend);

  filesize = ftell(fp);
  rewind(fp);

  f = (unsigned char *) calloc( (filesize+lines), sizeof(unsigned char) );
  carmen_test_alloc(f);
  l = (line_ref *) calloc( lines, sizeof(line_ref) );
  carmen_test_alloc(l);

  lines = 0; fend = 0;
  do{
    if ((s = fgets(&(f[fptr]),filesize,fp)) == NULL) {
      fend=1;
    } else {
      len = (strlen(s)+1);
      l[lines].posptr = fptr;
      if (sscanf( s, "%s", cmd ) == EOF) {
	  l[lines].time = 0;
      } else {
	  l[lines].time = getTimeStamp( s );
	  if ( !strcmp( cmd, "PARAM" ) || cmd[0] == '#' ) {
	      l[lines].time = 0.0;
	      l[lines].type = 0;
	  } else if ( !strcmp( cmd, "ODOM" )    ||
		      !strcmp( cmd, "TRUEPOS" ) ) {
	      l[lines].type = 0;
	  } else {
	      l[lines].type = 1;
	  }
      }
      fptr += len;
      lines++;
    }
  } while (!fend);
  fclose(fp);

  qsort( l, lines, sizeof(line_ref), lineCompare );

  for (i=0; i<lines; i++) {
    printf( "%s", &(f[l[i].posptr]) );
   }
  
  exit(0);
}
