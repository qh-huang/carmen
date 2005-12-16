#ifndef __INC_arcnet_h
#define __INC_arcnet_h

#ifdef __cplusplus
extern "C" {
#endif

typedef short unsigned int ArgContent;

typedef struct  
{
  unsigned short length;
  unsigned char argNumber;
  unsigned char argNumberRef;
  unsigned char *argData;    
} ArgBlock;

typedef struct {
  ArgBlock *(*procedure)();
  char *description;
} TableEntry;

extern TableEntry RPC_Table[];
extern TableEntry IRPC_Table[];
extern unsigned char d51_params[26];
extern unsigned char d51_net_map[32];

void *RPC_GetArg(unsigned char *error, short unsigned int expContent, 
		 ArgContent *content, 
		 unsigned char argNum, ArgBlock *args);

void RPC_PutArg(short unsigned int argType, short unsigned int dataLength, 
		char *data, ArgBlock *args);


unsigned char MAC_NoResponse();

unsigned char smc_in(unsigned char reg);

int ANET_Initialize();
void ANET_Poll();
void ANET_ConfSonar();

#ifdef __cplusplus
}
#endif

#endif /* __INC_arcnet_h */
