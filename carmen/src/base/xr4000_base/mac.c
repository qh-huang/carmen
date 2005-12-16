#include <stdio.h>
#include "arcnet.h"
#include "pc_d51.h"
#include "llc1.h"

struct LLC_MSG sap0; 
struct LLC_MSG sap1; 

unsigned char initParams(unsigned char addr)
{
  d51_params[0] = 1;
  d51_params[1] = 0;
  d51_params[2] = 0;
  d51_params[3] = 0;
  d51_params[4] = 0x87;
  d51_params[5] = 0;
  d51_params[6] = 0;
  d51_params[7] = 1;

  d51_params[8] = 0;
  d51_params[9] = 1;
  d51_params[10] = 1;
  d51_params[11] = 0x80;
  d51_params[12] = 1;
  d51_params[13] = 0;
  d51_params[14] = 0x18;
  d51_params[15] = 4;

  d51_params[16] = addr;
  d51_params[17] = 0x80;
  d51_params[18] = 0;
  d51_params[19] = 0;
  d51_params[20] = 0;
  d51_params[21] = 0;
  d51_params[22] = 0;
  return 0;
}

unsigned char initSaps()
{
  unsigned char status; 

  status = llc1_request(0, 0, 0xc, &sap0);
  if (status != 0)
    return 0xff;

  status = llc1_request(1, 0, 1, &sap1);
  if (status != 0)
    return 0xff;

  return 0;
}

unsigned char MAC_Init(unsigned char addr)
{
  unsigned char status; 

  initParams(addr);

  status = d51_init();
  if (status != 0) {
    printf("Core did not initialize properly\n");
    return 255;
  }
 
  status = initSaps();
  if (status != 0) {
    printf("Saps did not initialize properly\n");
    return 255;
  }
  return 0;
}

unsigned char MAC_NoResponse()
{
  return ((smc_in(0) >> 1) ^ 1) & 1;
}

unsigned char MAC_Receive(unsigned char *buffer, unsigned short *length, 
			  unsigned char *originAddr)
{
  *length = 0;
  sap1.msgptr = buffer;
  d51_check_int();
  llc1_service();
  switch (llc1_indication(1)) {

  case 3: 
    *length = (sap1.msbcount << 8) + sap1.lsbcount;
    *originAddr = sap1.dstation;
    break;

  case 1: 
    break;

  case 2: 
    break;

  case 0: 
    break;
  } 
  return 0;
}

unsigned char MAC_Send(unsigned char destAddr, unsigned char *message, 
		       unsigned short length)
{
  if (length > 508) 
    return 255;

  sap1.dstation = destAddr;
  sap1.msbcount = (length & 0xff00) >> 8;
  sap1.lsbcount = length & 0xff;
  sap1.msgptr = message;
  llc1_request(1, 1, 6, &sap1);
  return 0;
}

