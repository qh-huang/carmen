#ifndef RPC_H
#define RPC_H

#ifdef __cplusplus
extern "C" {
#endif

char RPC_SetInfo(unsigned char index1, char *string);


void RPC_SetTimeout(unsigned long nodePresent, unsigned long nodeResponding);

ArgContent RPC_NextFree(unsigned char argNum, ArgBlock *args);

unsigned char RPC_Init(unsigned char addr);

ArgBlock *RPC_Receive(unsigned char service, unsigned char *type,
		      unsigned char *sourceAddr, unsigned char *procIndex);

ArgBlock *RPC_Send(unsigned char destAddr, unsigned char service, 
		   unsigned char type, unsigned char procIndex, 
		   ArgBlock *args);

void RPC_PutArg(short unsigned int argType, short unsigned int dataLength, 
		char *data, ArgBlock *args);

void *RPC_GetArg(unsigned char *error, short unsigned int expContent, 
		 ArgContent *content, 
		 unsigned char argNum, ArgBlock *args);

void RPC_PutINT8(char c, ArgBlock *args);

void RPC_PutINT16(short int i, ArgBlock *args);

void RPC_PutINT32(long int i, ArgBlock *args);

#ifdef __cplusplus
}
#endif

#endif
