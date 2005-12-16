#ifndef LLC1_H
#define LLC1_H

#ifdef __cplusplus
extern "C" {
#endif

struct LLC_MSG
{
  unsigned char event;
  unsigned char dstation;
  unsigned char ssap;
  unsigned char dsap;
  unsigned char group;
  unsigned char control;
  unsigned char msbcount;
  unsigned char lsbcount;
  unsigned char *msgptr;
};

unsigned char llc1_service();
unsigned char llc1_request(unsigned char lssap, unsigned char ldsap, 
			   unsigned char event, struct LLC_MSG *request);
unsigned char llc1_indication(unsigned char lsap);

#ifdef __cplusplus
}
#endif

#endif
