#include <carmen/carmen.h>

#define LOGFILE

void publish_params_from_logfile(FILE *fp)
{
  char line[2000], *err, tag[1024], param[1024], value[1024];
  int count = 0;
  int param_err;

  err = fgets(line, 2000, fp);
  while(err != NULL) {
    if(line[0] != '#') {
      sscanf(line, "%s", tag);
        if(strcmp(tag, "PARAM") == 0) {
	  sscanf(line, "%*s %s %s", param, value);
	  param_err = carmen_param_set_variable(param, value, NULL);
	  if (param_err == -1)
	    exit(-1);
	  count++;
	}
    }
    err = fgets(line, 2000, fp);
  }

  carmen_warn("Set %d parameters\n", count);
}

void publish_params_from_paramfile(FILE *fp, char *param_label, char *filename)
{
  char line[2000], *err, param[1024], value[1024];
  int count = 0;
  int param_err;
  int skipping = 0;
  int line_num = 0;
  char *mark, *token;
  int token_num;
  int line_length;

  err = fgets(line, 2000, fp);
  for (; err != NULL; err = fgets(line, 2000, fp)) {
    line_num++;
    line[1999] = '\0';
    if (strlen(line) == 1999) 
      carmen_die("Line %d of file %s is too long.\n"
		 "Maximum line length is %d. Please correct this line.\n\n"
		 "It is also possible that this file has become corrupted.\n"
		 "Make sure you have an up-to-date version of carmen, and\n"
		 "consult the param_server documentation to make sure the\n"
		 "file format is valid.\n", line_num, filename, 2000);
    
    if (feof(fp))
      break;
    mark = strchr(line, '#');    /* strip comments and trailing returns */
    if (mark != NULL)
      mark[0] = '\0';
    mark = strchr(line, '\n');
    if (mark != NULL)
      mark[0] = '\0';
    
    // Trim off trailing white space 
    
    line_length = strlen(line) - 1;
    while (line_length >= 0 && 
	   (line[line_length] == ' ' || line[line_length] == '\t' ))
      line[line_length--] = '\0';
  
    line_length++;
    
    if (line_length == 0)
      continue;
    
    // Skip over initial blank space
    
    mark = line + strspn(line, " \t");
    if (strlen(mark) == 0) 
      carmen_die("You have encountered a bug in carmen. Please report it\n"
		 "to the carmen maintainers. \n"
		 "Line %d, function %s, file %s\n", __LINE__, __FUNCTION__,
		 __FILE__);
    
    token_num = 0;
    
    /* tokenize line */
    token = mark;
    
    // Move mark to the first whitespace character.
    mark = strpbrk(mark, " \t");
    // If we found a whitespace character, then turn it into a NULL
    // and move mark to the next non-whitespace.
    if (mark) 
      {
	mark[0] = '\0';
	mark++;
	mark += strspn(mark, " \t");
      }
    
    if (strlen(token) > 254) 
      {
	carmen_warn("Bad file format of %s on line %d.\n"
		    "The parameter name %s is too long (%d characters).\n"
		    "A parameter name can be no longer than 254 "
		    "characters.\nSkipping this line.\n", filename, 
		    count, token, strlen(token));
	continue;
      }
    
    strcpy(param, token);
    token_num++;
    
    // If mark points to a non-whitespace character, then we have a
    // two-token line
    if (mark)
      {
	if (strlen(mark) > 1999) 
	  {
	    carmen_warn("Bad file format of %s on line %d.\n"
			"The parameter value %s is too long (%d "
			"characters).\nA parameter value can be no longer "
			"than %d characters.\nSkipping this line.\n", 
			filename, count, mark, strlen(mark),
			1999);
	    continue;
	  }
	strcpy(value, mark);
	token_num++;
      }
    
    if (param[0] == '[') 
      {
	if (param[1] == '*')
	  skipping = 0;
	else if (param_label != NULL && 
	    carmen_strncasecmp(param+1, param_label, strlen(param_label)) == 0)
	  skipping = 0;	    
	else
	  skipping = 1;
	continue;
      }
    else if(token_num == 2 && !skipping) 
      {
	param_err = carmen_param_set_variable(param, value, NULL);
	if (param_err < 0)
	  carmen_warn("Couldn't set parameter %s\n", param);
	else
	  count++;
      }
  } 
  
  carmen_warn("Set %d parameters\n", count);
}


int 
main(int argc, char *argv[]) 
{
  char buffer[1024];
  char *param_label = NULL, *tmp;
  FILE *fp;
  int index;

  if (argc < 2)
    carmen_die("Usage: %s [-p paramset] {logfile | paramfile}\n", argv[0]);
  
  carmen_initialize_ipc(argv[0]);

  for (index = 1; index < argc; index++) {
    if (strcmp(argv[index], "-p") == 0) {
      if (index == argc-1)
	carmen_die("Usage: %s [-p paramset] {logfile | paramfile}\n", argv[0]);
      param_label = argv[index+1];
      tmp = argv[index];
      argv[index] = argv[1];
      argv[1] = tmp;
      tmp = argv[index+1];
      argv[index+1] = argv[2];
      argv[2] = tmp;
      index = 3;
      break;
    }
  }

  if (param_label && index != argc-1)
    carmen_die("Usage: %s [-p paramset] {logfile | paramfile}\n", argv[0]);
  
  if (!param_label && index != 2)
    carmen_die("Usage: %s [-p paramset] {logfile | paramfile}\n", argv[0]);

  if (!param_label)
    index--;

  if (!carmen_file_exists(argv[index])) {
    carmen_warn("%s does not exist.\n", argv[index]);
    carmen_die("Usage: %s [-p paramset] {logfile | paramfile}\n", argv[0]);
  }
    
  fp = fopen(argv[index], "r");
  if (fp == NULL) 
    carmen_die_syserror("Error reading from %s\n", argv[1]);

  buffer[1023] = 0;
  if (fgets(buffer, 1023, fp) == NULL)
    carmen_die("Unexpected end of file on %s\n", argv[1]);
  rewind(fp);

  if (strncmp(buffer, CARMEN_LOGFILE_HEADER, 
	      strlen(CARMEN_LOGFILE_HEADER)) == 0) {
    if (param_label)
      carmen_warn("Ignoring paramset argument for reading from logfile.\n");
    publish_params_from_logfile(fp);    
  } else {
    publish_params_from_paramfile(fp, param_label, argv[index]);
  }

  exit(0);
}
