#ifndef MAC_H
#define MAC_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned char MAC_Receive(unsigned char *buffer, unsigned short *length, 
			  unsigned char *originAddr);

unsigned char MAC_Init(unsigned char addr);

unsigned char MAC_Send(unsigned char destAddr, unsigned char *message, 
		       unsigned short length);

#ifdef __cplusplus
}
#endif

#endif
