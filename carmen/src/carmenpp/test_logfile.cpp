#include "cpp_carmen.h"

#include "cpp_laser.h"
#include "cpp_base.h"
#include "cpp_robot.h"
#include "cpp_simulator.h"
#include "cpp_logfile.h"

int main(int argc, char** argv) {

  if (argc != 2)
    carmen_die("SYNTAX: %s <carmen log file>\n", argv[0]);
  
  carmen_FILE *carmen_stdout = new carmen_FILE;
  carmen_stdout->compressed = 0;
  carmen_stdout->fp = stdout;
  
  LogFile log;
  log.load(argv[1]);
  
  for (LogFile::Collection::const_iterator it = log.begin();
       it != log.end(); ++it) {    
    (*it)->save(carmen_stdout, (*it)->getTimestamp());
  }
  
  return 0;
}
