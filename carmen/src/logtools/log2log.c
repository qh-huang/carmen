#include <carmen/carmen.h>
#include <carmen/logtools.h>

void
print_usage( void )
{
  fprintf( stderr, "\nusage: log2log <LOG-FILE> <LOG-FILE>\n");
}

/**************************************************************************
 * MAIN-LOOP
 **************************************************************************/

int
main(int argc, char *argv[])
{
  logtools_log_data_t   log;
  int                   i;

  if (argc<3) {
    print_usage();
    exit(1);
  }
  
  for (i=1; i<argc-2; i++) {
    {
      print_usage();
      exit(1);
    }
  }
  
  if (!logtools_read_logfile( &log, argv[argc-2] ))
      exit(1);

  if (!logtools_write_logfile( &log, argv[argc-1] ))
      exit(1);
  
  exit(0);
}
