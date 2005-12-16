#ifndef __INC_setup_h
#define __INC_setup_h

#ifdef __cplusplus
extern "C" {
#endif

char *SETUP_GetValue(char *Key);
char *SETUP_ExtGetValue(char *Section, char *Key);

unsigned char SETUP_ReadSetup(char *filename);

#ifdef __cplusplus
}
#endif

#endif
