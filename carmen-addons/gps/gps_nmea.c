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
 * Public License along with Foobar; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

/*********************************************************
This file contains a code to read from/write to GPS in 
NMEA 0183 ASCII format 
**********************************************************/

#include <carmen/carmen.h>
#include "gps_nmea.h"

static char buffer[GPS_NMEA_BUFFERSIZE];
static int  bufferind = 0;
static int bufferfill = 0;


//returns the number of chars on the port
long serialCharsAvail(int ttyfp)
{
  int count;
  ioctl(ttyfp, FIONREAD, &count);
  return (long) count;
}

long TickCount()
{
  time_t count;

  time(&count);
  return (long) count;
}

//tests that the protocol of the GPS is valid
//Returns 0 if successful, returns -1 otherwise
int carmen_gps_nmeacheck(int ttyfp)
{
  unsigned char c;
  int ret;
  long tttime = TickCount();

  //wait for a message
  while(1) 
    {
      /* timeout exceded */
      if (TickCount() > (tttime + (long)GPS_CHECK_TIMEOUT)) {
	return -1;
      }

      while (serialCharsAvail(ttyfp) && bufferind < GPS_NMEA_BUFFERSIZE-10) {
	//read in the next character
	ret = read(ttyfp, &c, 1);
	
	//reset the buffer index
	if(c == '$')
	  {
	    bufferind = 0;
	  }
	
	//store the data
	buffer[bufferind] = c;
	buffer[bufferind+1] = '\0';
	bufferind++;
	
	if(strcmp(buffer, GPSNMEARecPosn) == 0)
	  return 0;
      }
    }
  
  return -1;
}

//copies into the GPS buffer the passed in buffer of size N
void copy_buffer(char* inbuffer, int N)
{
  int i;

  //note that the buffer got filled
  if(bufferind + N >= GPS_NMEA_BUFFERSIZE)
    bufferfill = 1;

  for(i = 0; i < N; i++)
    {
      buffer[bufferind] = inbuffer[i];
      bufferind = (bufferind+1)%GPS_NMEA_BUFFERSIZE;
    }

}


//gets new information from GPS and writes it into the buffer
//returns 0 if successful, -1 otherwise
int write_buffer(int ttyfp)
{
  int nChars, nReadChars;
  char TempBuffer[GPS_NMEA_BUFFERSIZE];
  
  //how many chars are available
  nChars = serialCharsAvail(ttyfp);

  //return if there is nothing available
  if(nChars == 0)
    return 0;

  //limit it
  if(nChars > GPS_NMEA_BUFFERSIZE-1)
    nChars = GPS_NMEA_BUFFERSIZE-1;

  //read them in
  nReadChars = read(ttyfp, &TempBuffer, nChars);

  //incorrect reading
  if(nReadChars <= 0)
    return -1;

  //copy into the buffer
  copy_buffer(TempBuffer, nReadChars);

  return 0;
}

//returns previous buffer index. returns -1 if end of the buffer is reached
int GetPrevBufInd(int locbuffind)
{

  if(locbuffind == bufferind+1)
    return -1;

  if(locbuffind > bufferind+1)
    return locbuffind-1;

  if(locbuffind > 0)
    return locbuffind-1;

  if(locbuffind == 0 && bufferfill == 1)
    return GPS_NMEA_BUFFERSIZE-1;
  
  return -1;
}

//returns 1 if the header is correct of the buffer stored at locbuffind index
//returns 0 otherwise
int IsCorrectHeader(int locbufind)
{
  char Header[1024];
  int len = strlen(GPSNMEARecPosn);
  int i;

  strcpy(Header, GPSNMEARecPosn);

  for(i = 0; i < len; i++)
    {
      if(buffer[locbufind] != Header[i])
	return 0;
      
      //increment buffer index
      locbufind = (locbufind+1)%GPS_NMEA_BUFFERSIZE;
    }

  return 1;
}

//copies a message into the given buffer
void copy_message(char* inbuffer, int locbufind)
{
  int i;

  for(i = 0; i < GPS_NMEA_BUFFERSIZE; i++)
    {
      //copy the next char
      inbuffer[i] = buffer[locbufind];
	
      //increment the pointer
      locbufind = (locbufind+1)%GPS_NMEA_BUFFERSIZE;

      //check whether the end of the message is reached
      if(buffer[locbufind] == '$')
	return;
    }

}


//processes the data in the buffer and extracts the latest position message
void extract_message(char* Message)
{
  int i = 0, locbufind;
  int bVeryLast = 1;

  //initialize to an empyt message
  Message[0] = '\0';

  for(i = 0, locbufind = GetPrevBufInd(bufferind); 
      i < GPS_NMEA_BUFFERSIZE; i++)
    {
      //end of the buffer reached
      if(locbufind == -1)
	return;

      //start of a new message is detected
      if(buffer[locbufind] == '$')
	{
	  //disregard the very last message since it may not be complete
	  if(bVeryLast)
	    bVeryLast = 0;
	  //otherwise it might be what we need
	  else if(IsCorrectHeader(locbufind))
	    {
	      //copy the message
	      copy_message(Message, locbufind);
	      return;
	    }
	}

      //increment buffer index
      locbufind = GetPrevBufInd(locbufind);
    }
}

//skips the current field
//returns 0 on success and -1 otherwise
int read_uptonextfield(char* Message, int *pMessageIndex)
{
  int i;

  for(i = 0; i < 100; i++)
    {
      if(Message[*pMessageIndex] == ',')
	{
	  (*pMessageIndex)++;
	  return 0;
	}
      else if(Message[*pMessageIndex] == '\0')
	{
	  return -1;
	}
      
      *pMessageIndex = *pMessageIndex + 1;

    }

  return -1;
}

//return 0 on success and -1 on failure
int process_message(char* Message, double* plat, double *plong, int *psatnum, 
			double *precdil)
{
  int MessageIndex = 0;
  float min;
  int deg;
  int fix;
  float temp;

  //printf("--**%s**--\n", Message);

  //skip the header and time
  MessageIndex = 0;
  if(read_uptonextfield(Message, &MessageIndex) < 0)
    {
      carmen_warn("gps sent an invalid message\n");
      return -1;
    }
  if(read_uptonextfield(Message, &MessageIndex) < 0)
    {
      carmen_warn("gps sent an invalid message\n");
      return -1;
    }

  //read in the latitude
  sscanf(&Message[MessageIndex], "%2d", &deg);
  MessageIndex += 2;
  sscanf(&Message[MessageIndex], "%f", &min);
  *plat = deg + min/60.0;
 
  //skip the latitude
  if(read_uptonextfield(Message, &MessageIndex) < 0)
    {
      carmen_warn("gps sent an invalid message\n");
      return -1;
    }
 
  //skip the hemishpere
  if(read_uptonextfield(Message, &MessageIndex) < 0)
    {
      carmen_warn("gps sent an invalid message\n");
      return -1;
    }

  //read in the longitude
  sscanf(&Message[MessageIndex], "%3d", &deg);
  MessageIndex += 3;
  sscanf(&Message[MessageIndex], "%f", &min);
  *plong = deg + min/60.0;

  //skip the longitude
  if(read_uptonextfield(Message, &MessageIndex) < 0)
    {
      carmen_warn("gps sent an invalid message\n");
      return -1;
    }

  //skip the hemishpere
  if(read_uptonextfield(Message, &MessageIndex) < 0)
    {
      carmen_warn("gps sent an invalid message\n");
      return -1;
    }

  //read in fix or not
  sscanf(&Message[MessageIndex], "%d", &fix);

  //skip the fix
  if(read_uptonextfield(Message, &MessageIndex) < 0)
    {
      carmen_warn("gps sent an invalid message\n");
      return -1;
    }
  
  //read in the number of satellites
  sscanf(&Message[MessageIndex], "%d", psatnum);

  //skip the number of satellites
  if(read_uptonextfield(Message, &MessageIndex) < 0)
    {
      carmen_warn("gps sent an invalid message\n");
      return -1;
    }
  
  //read in the precision
  if(fix > 0)
    {
      sscanf(&Message[MessageIndex], "%f", &temp);
      *precdil = temp;
    }
  else
    *precdil = 0;

  //printf("lat=%f long=%f fix=%d satnum=%d precdil=%f\n", *plat, *plong, fix,
  // *psatnum, *precdil);

  return 0;
}

//runs a new cycle of processing GPS that is configured in NMEA mode
//returns 0 if successful, -1 otherwise
int carmen_gps_run_nmea(int ttyfp, double* plat, double *plong, int *psatnum, 
			double *precdil)
{
  char Message[1024];

  //write new data into the buffer
  if(write_buffer(ttyfp) < 0)
    {
      carmen_warn("\nerror reading from gps and writing into a buffer\n");
      return -1;
    }

  //extract the last position message from the buffer
  extract_message(Message);
  if(strlen(Message) == 0 && bufferfill == 1)
    {
      carmen_warn("\nbuffer is full but no position message from gps\n");
      return -1;
    }


  //process the message
  if(strlen(Message) > 0)
    process_message(Message, plat, plong, psatnum, precdil);
  else
    {
      *plat = 0;
      *plong = 0;
      *psatnum = 0;
      *precdil = 0;
    }
  

  return 0;
}

