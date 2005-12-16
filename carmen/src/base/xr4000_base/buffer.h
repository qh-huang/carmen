#ifndef __INC_buffer_h
#define __INC_buffer_h

#ifdef __cplusplus
extern "C" {
#endif

void BUF_AddString(char *s);
char *BUF_Buffer();
void BUF_Reset(char *Buf);
long int BUF_Index();
void BUF_AddChar(char c);
void BUF_AddSpace();
void BUF_AddBoolean(unsigned char b);
void BUF_AddLong(long l);
void BUF_AddDouble(double d);
void BUF_AddPrecisionDouble(double d);

#ifdef __cplusplus
}
#endif

#endif /* __INC_buffer_h */
