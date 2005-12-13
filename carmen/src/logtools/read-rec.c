#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <fnmatch.h>

#include <carmen/logtools.h>

#include "defines.h"

int
gsm_compare( const void *a, const void *b )
{
  static GSM_CELL_INFO v1, v2;
  v1 = *(const GSM_CELL_INFO *)a; 
  v2 = *(const GSM_CELL_INFO *)b;
  return(v1.rxl<v2.rxl);
}


void
placelab_sort_gsm( REC2_DATA * rec )
{
  int i;
  for (i=0; i<rec->numgsm; i++) {
    qsort( rec->gsm[i].cell, rec->gsm[i].numcells,
	   sizeof(GSM_CELL_INFO), gsm_compare );
  }
}

int
read_data2d_file( REC2_DATA * rec, char * filename )
{
  enum FILE_TYPE     inp_type = UNKOWN;
  char               fname[MAX_NAME_LENGTH];

    fprintf( stderr, "#####################################################################\n" );
  if ( !fnmatch( "script:*", filename, 0) ) {
    fprintf( stderr, "# INFO: use script-file-type!\n" );
    strncpy( fname, &(filename[7]), MAX_NAME_LENGTH );
    inp_type = SCRIPT;
  } else if ( !fnmatch( "rec:*", filename, 0) ) {
    fprintf( stderr, "# INFO: read rec-file-type!\n" );
    strncpy( fname, &(filename[4]), MAX_NAME_LENGTH );
    inp_type = REC;
  } else if ( !fnmatch( "carmen:*", filename, 0) ) {
    fprintf( stderr, "# INFO: read carmen-file-type!\n" );
    strncpy( fname, &(filename[7]), MAX_NAME_LENGTH );
    inp_type = CARMEN;
  } else if ( !fnmatch( "moos:*", filename, 0) ) {
    fprintf( stderr, "# INFO: read moos-file-type!\n" );
    strncpy( fname, &(filename[5]), MAX_NAME_LENGTH );
    inp_type = MOOS;
  } else if ( !fnmatch( "player:*", filename, 0) ) {
    fprintf( stderr, "# INFO: read player-file-type!\n" );
    strncpy( fname, &(filename[7]), MAX_NAME_LENGTH );
    inp_type = PLAYER;
  } else if ( !fnmatch( "placelab:*", filename, 0) ) {
    fprintf( stderr, "# INFO: read placelab-file-type!\n" );
    strncpy( fname, &(filename[9]), MAX_NAME_LENGTH );
    inp_type = PLACELAB;
  } else if ( !fnmatch( "plab:*", filename, 0) ) {
    fprintf( stderr, "# INFO: read placelab-file-type!\n" );
    strncpy( fname, &(filename[5]), MAX_NAME_LENGTH );
    inp_type = PLACELAB;
  } else if ( !fnmatch( "*" FILE_SCRIPT_EXT, filename, 0) ) {
    fprintf( stderr, "# INFO: use script-file-type!\n" );
    strncpy( fname, filename, MAX_NAME_LENGTH );
    inp_type = SCRIPT;
  } else if ( !fnmatch( "*" FILE_REC_EXT, filename, 0) ) {
    fprintf( stderr, "# INFO: read rec-file-type!\n" );
    strncpy( fname, filename, MAX_NAME_LENGTH );
    inp_type = REC;
  } else if ( !fnmatch( "*"FILE_CARMEN_EXT, filename, 0) ) {
    fprintf( stderr, "# INFO: read carmen-file-type!\n" );
    strncpy( fname, filename, MAX_NAME_LENGTH );
    inp_type = CARMEN;
  } else if ( !fnmatch( "*" FILE_MOOS_EXT, filename, 0) ) {
    fprintf( stderr, "# INFO: read moos-file-type!\n" );
    strncpy( fname, filename, MAX_NAME_LENGTH );
    inp_type = MOOS;
  } else if ( !fnmatch( "*" FILE_PLAYER_EXT, filename, 0) ) {
    fprintf( stderr, "# INFO: read player-file-type!\n" );
    strncpy( fname, filename, MAX_NAME_LENGTH );
    inp_type = PLAYER;
  } else if ( !fnmatch( "*" FILE_PLACELAB_EXT, filename, 0) ) {
    fprintf( stderr, "# INFO: read placelab-file-type!\n" );
    strncpy( fname, filename, MAX_NAME_LENGTH );
    inp_type = PLACELAB;
  } else if ( !fnmatch( "*" FILE_SAPHIRA_EXT, filename, 0) ) {
    fprintf( stderr, "# INFO: read saphira-file-type!\n" );
    strncpy( fname, filename, MAX_NAME_LENGTH );
    inp_type = SAPHIRA;
  } else {
    fprintf( stderr, "# INFO: read carmen-file-type!\n" );
    strncpy( fname, filename, MAX_NAME_LENGTH );
    inp_type = CARMEN;
  }

  switch (inp_type) {
  case SCRIPT:
    if (read_script( fname, rec, 1 ) !=0 )
      return(FALSE);
    break;
  case REC:
    if (load_rec2d_file( fname, rec, REC, READ_MODE_DONT_STOP ) !=0 )
      return(FALSE);
    break;
  case CARMEN:
    if (load_rec2d_file( fname, rec, CARMEN, READ_MODE_DONT_STOP ) !=0 )
      return(FALSE);
    break;
  case MOOS:
    if (load_rec2d_file( fname, rec, MOOS, READ_MODE_DONT_STOP ) !=0 )
      return(FALSE);
    break;
  case PLAYER:
    if (load_rec2d_file( fname, rec, PLAYER, READ_MODE_DONT_STOP ) !=0 )
      return(FALSE);
    break;
  case SAPHIRA:
    if (load_rec2d_file( fname, rec, SAPHIRA, READ_MODE_DONT_STOP ) !=0 )
      return(FALSE);
    break;
  case PLACELAB:
    if (load_rec2d_file( fname, rec, PLACELAB, READ_MODE_DONT_STOP ) !=0 )
      return(FALSE);
    break;
  default:
    fprintf( stderr, "ERROR: unknown file-type!\n" );
    return(FALSE);
  }

  return(TRUE);
}

/***********************************************************************/
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int
read_rec2d_file( char *filename, REC2_DATA *rec,  int mode )
{
  return(load_rec2d_file( filename, rec, REC, mode ));
}

int
load_rec2d_file( char *filename, REC2_DATA *rec, enum FILE_TYPE type, int mode )
{

  char      line[MAX_LINE_LENGTH];
  int       FEnd, numPos, numCorrPos, numScan, numDynProb;
  char      dummy[MAX_CMD_LENGTH];
  char      command[MAX_CMD_LENGTH];
  char    * sptr, * iline, * lptr;
  FILE    * iop;
  int       linectr = 0, corrposctr = 0, posctr = 0, poscorrctr = 0;
  int       laserctr = 0, dynctr = 0, estctr = 0, gpsctr = 0, compassctr = 0;
  int       numEntries = 0, markerctr = 0, rfidctr = 0, pos3dctr = 0;
  int       wifictr = 0, gsmctr = 0;

  fprintf( stderr, "# read file %s ...\n", filename );
  if ((iop = fopen( filename, "r")) == 0){
    fprintf(stderr, "# WARNING: no file %s\n", filename );
    return(-1);
  }

  FEnd = 0;

  rec->info.system = type;

  switch (rec->info.system) {
    
  case REC:
    do{
      linectr++;
      if (fscanf(iop, "%s", command) == EOF)
	FEnd=1;
      else{
	if (!strcmp( command, "POS" )) {
	  posctr++;
	} else if (!strcmp( command, "CORR-POS" )) {
	  corrposctr++;
	} else if (!strcmp( command, "POS-CORR" )) {
	  poscorrctr++;
	} else if ( (!strcmp( command, "LASER" ))       ||
		    (!strcmp( command, "LASER-RANGE" )) ||
		    (!strcmp( command, "LASER-SCAN" ))  ||
		    (!strcmp( command, "CARMEN-LASER" )) ) {
	  laserctr++;
	} else if (!strcmp( command, "HELI-POS" )) {
	  pos3dctr++;
	} else if (!strcmp( command, "DYNAMIC-PROB" )) {
	  dynctr++;
	} else if (!strcmp( command, "GPS" )) {
	  gpsctr++;
	} else if (!strcmp( command, "NMEA-GGA" )) {
	  gpsctr++;
	} else if ( (!strcmp( command, "COMPASS2D" )) ||
		    (!strcmp( command, "COMPASS3D" )) ) {
	  compassctr++;
	} else if ( (!strcmp( command, "MARK-POS")) ||
		    (!strcmp( command, "MARKER")) ) {
	  markerctr++;
	} else if ( (!strcmp( command, "RFID")) ||
		    (!strcmp( command, "RFID-SIEMENS")) ) {
	  rfidctr++;
	} else if ( !strcmp( command, "WIFI") ||
		    !strcmp( command, "WIFI-DIST") ) {
	  wifictr++;
	} else if ( !strcmp( command, "GSM-E2EMM") ) {
	  gsmctr++;
	} else if ( (!strcmp( command, "RFID")) ||
		    (!strcmp( command, "RFID-SIEMENS")) ) {
	  rfidctr++;
	}
	fgets(command,sizeof(command),iop);
      }
    } while (!FEnd);
    break;

  case CARMEN:
    do{
      linectr++;
      if (fscanf(iop, "%s", command) == EOF)
	FEnd=1;
      else{
	if (!strcmp( command, "ODOM")) {
	  posctr++;
	} else if (!strcmp( command, "FLASER") ){
	  laserctr++;
	} else if (!strcmp( command, "RLASER") ){
	  laserctr++;
	} else if (!strcmp( command, "MARKER")) {
	  markerctr++;
	}
	fgets(command,sizeof(command),iop);
      }
    } while (!FEnd);
    break;
    
  case MOOS:
    do{
      linectr++;
      if (fscanf( iop, "%s %s", dummy, command ) == EOF) {
	FEnd=1;
      } else{
	if (!strcmp( command, "ROBOT_POSITION")) {
	  posctr++;
	} else if (!strcmp( command, "LASER_RANGE") ){
	  laserctr++;
	}
	fgets(command,sizeof(command),iop);
      }
    } while (!FEnd);
    break;

  case PLAYER:
    do{
      linectr++;
      if (fgets(line,MAX_LINE_LENGTH,iop) == NULL) {
	FEnd=1;
      } else {
	if (strstr( line, "position" ) != NULL) {
	  posctr++;
	} else if (strstr( line, "laser" ) != NULL) {
	  laserctr++;
	}
      }
    } while (!FEnd);
    break;

  case SAPHIRA:
    do{
      linectr++;
      if (fscanf( iop, "%s", command ) == EOF) {
	FEnd=1;
      } else{
	if (!strcmp( command, "robot:")) {
	  posctr++;
	} else if (!strcmp( command, "sick1:") ){
	  laserctr++;
	}
	fgets(command,sizeof(command),iop);
      }
    } while (!FEnd);
    break;

  case PLACELAB:
    iline = (char *) malloc( MAX_LINE_LENGTH * sizeof(char) );
    while (fgets(iline,MAX_LINE_LENGTH,iop) != NULL) {
      lptr = iline; linectr++;
      do {
	sptr = strsep( &lptr, "|" );
	if (sptr != NULL) {
	  sscanf( sptr, "%[^=]", command );
	  if (!strncasecmp( "type", command, MAX_LINE_LENGTH)) {
	    sscanf( sptr, "%[^=]=%[^|]", dummy, command );
	    if (!strncasecmp( "gsm", command, MAX_LINE_LENGTH)) {
	      gsmctr++;
	      break;
	    } else if (!strncasecmp( "gps", command, MAX_LINE_LENGTH)) {
	      gpsctr++;
	      break;
	    } else if (!strncasecmp( "marker", command, MAX_LINE_LENGTH)) {
	      markerctr++;
	      break;
	    } else if (!strncasecmp( "estimate", command, MAX_LINE_LENGTH)) {
	      estctr++;
	      break;
	    }
	  }
	}
      } while (sptr != NULL); 
    }
    break;
  default:
    fprintf( stderr, "ERROR: unknown file-type!\n" );
    return(FALSE);

  }
  
  if (mode && READ_MODE_VERBOSE) {
    fprintf( stderr, "#####################################################################\n" );
    if (posctr>0)
      fprintf( stderr, "# found %d positions\n", posctr );
    if (laserctr>0)
      fprintf( stderr, "# found %d laserscans\n", laserctr );
    if (pos3dctr>0)
      fprintf( stderr, "# found %d positions3d\n", pos3dctr );
    if (poscorrctr>0)
      fprintf( stderr, "# found %d pos corr\n", poscorrctr );
    if (corrposctr>0)
      fprintf( stderr, "# found %d corr pos\n", corrposctr );
    if (dynctr>0)
      fprintf( stderr, "# found %d dynamic probs\n", dynctr );
    if (compassctr>0)
      fprintf( stderr, "# found %d compass pos\n", compassctr );
    if (gpsctr>0)
      fprintf( stderr, "# found %d gps pos\n", gpsctr );
    if (markerctr>0)
      fprintf( stderr, "# found %d marker\n", markerctr );
    if (rfidctr>0)
      fprintf( stderr, "# found %d rfid\n", rfidctr );
    if (wifictr>0)
      fprintf( stderr, "# found %d wifi\n", wifictr );
    if (gsmctr>0)
      fprintf( stderr, "# found %d gsm\n", gsmctr );
    if (estctr>0)
      fprintf( stderr, "# found %d estimates\n", estctr );
  }
  
  numEntries =
    posctr + poscorrctr + corrposctr + laserctr + dynctr +
    gpsctr + compassctr + markerctr + rfidctr + pos3dctr +
    wifictr + gsmctr + estctr + 1;

  rec->numentries = 0;

  rec->entry   =
    (ENTRY_POSITION *) malloc( numEntries * sizeof(ENTRY_POSITION) );

  rewind(iop);

  if (posctr>0) {
    rec->psens =
      (POSSENS2_DATA *) malloc( posctr * sizeof(POSSENS2_DATA) );
  } else
    rec->psens = NULL;

  if (pos3dctr>0) {
    rec->psens3d =
      (POSSENS3_DATA *) malloc( pos3dctr * sizeof(POSSENS3_DATA) );
  } else
    rec->psens3d = NULL;

  if (corrposctr>0)
    rec->cpsens =
      (POSSENS2_DATA *) malloc( corrposctr * sizeof(POSSENS2_DATA) ); 
  else
    rec->cpsens = NULL;

  if (poscorrctr>0)
    rec->poscorr =
      (POS_CORR2_DATA *) malloc( poscorrctr * sizeof(POS_CORR2_DATA) ); 
  else
    rec->poscorr = NULL;

  if (laserctr>0)
    rec->lsens =
      (LASERSENS2_DATA *) malloc( laserctr * sizeof(LASERSENS2_DATA) ); 
  else
    rec->lsens = NULL;

  if (gpsctr>0)
    rec->gps =
      (GPS_DATA *) malloc( gpsctr * sizeof(GPS_DATA) );
  else
    rec->gps = NULL;

  if (compassctr>0)
    rec->compass =
      (COMPASS3_DATA *) malloc( compassctr * sizeof(COMPASS3_DATA) );
  else 
    rec->compass = NULL;

  if (markerctr>0)
    rec->marker =
      (MARKER_DATA *) malloc( markerctr * sizeof(MARKER_DATA) );
  else 
    rec->marker = NULL;
  
  if (rfidctr>0)
    rec->rfid =
      (RFID_DATA *) malloc( rfidctr * sizeof(RFID_DATA) );
  else 
    rec->rfid = NULL;
  
  if (gsmctr>0)
    rec->gsm =
      (GSM_DATA *) malloc( gsmctr * sizeof(GSM_DATA) );
  else 
    rec->gsm = NULL;
   
  if (wifictr>0)
    rec->wifi =
      (WIFI_DATA *) malloc( wifictr * sizeof(WIFI_DATA) );
  else 
    rec->wifi = NULL;
  
  if (estctr>0)
    rec->est =
      (EST_DATA *) malloc( estctr * sizeof(EST_DATA) );
  else 
    rec->est = NULL;
  
  numScan    = 0;
  numPos     = 0;
  numCorrPos = 0;
  numDynProb = 0;

  rec->numpositions    = 0;
  rec->numpositions3d  = 0;
  rec->numposcorr      = 0;
  rec->numcpositions   = 0;
  rec->numlaserscans   = 0;
  rec->numgps          = 0;
  rec->numcompass      = 0;
  rec->nummarkers      = 0;
  rec->numrfid         = 0;
  rec->numgsm          = 0;
  rec->numwifi         = 0;
  rec->numestimates    = 0;
  
  FEnd    = 0;
  linectr = 0;


  switch (rec->info.system) {
    
  case REC:
    do{
      if (fgets(line,MAX_LINE_LENGTH,iop) == NULL) {
	break;
      }
      rec->entry[rec->numentries].linenr = linectr++;
    } while (rec2_parse_line( line, rec, TRUE, TRUE ));
    break;

  case CARMEN:
    do{
      if (fgets(line,MAX_LINE_LENGTH,iop) == NULL) {
	break;
      }
    } while (carmen_parse_line( line, rec, TRUE, TRUE ));
    break;

  case MOOS:
    do{
      if (fgets(line,MAX_LINE_LENGTH,iop) == NULL) {
	break;
      }
    } while (moos_parse_line( line, rec, TRUE, TRUE ));
    break;

  case PLAYER:
    do{
      if (fgets(line,MAX_LINE_LENGTH,iop) == NULL) {
	break;
      }
    } while (player_parse_line( line, rec, TRUE, TRUE ));
    break;

  case SAPHIRA:
    do{
      if (fgets(line,MAX_LINE_LENGTH,iop) == NULL) {
	break;
      }
    } while (saphira_parse_line( line, rec, TRUE, TRUE ));
    break;

  case PLACELAB:
    {
      do{
	if (fgets(line,MAX_LINE_LENGTH,iop) == NULL) {
	  break;
	}
      } while (placelab_parse_line( line, rec, TRUE, TRUE ));
    }
    placelab_sort_gsm( rec );
    break;

  default:
    fprintf( stderr, "ERROR: unknown file-type!\n" );
    return(FALSE);

  }
  
  if (0 && mode && READ_MODE_VERBOSE) {
    fprintf( stderr, "#####################################################################\n" );
    fprintf( stderr, "# num positions     = %d\n",
	     rec->numpositions );
    fprintf( stderr, "# num laserscans    = %d\n",
	     rec->numlaserscans );
    fprintf( stderr, "#####################################################################\n" );
  }

  fclose(iop);
  return(0);
}

