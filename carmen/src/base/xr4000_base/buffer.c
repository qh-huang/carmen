#include <carmen/carmen.h>
#include <assert.h>

#include "buffer.h"
#define FIXED_BUF_SIZE 800000

static long unsigned int BufferSize = FIXED_BUF_SIZE; 
static long unsigned int Index = 0; 
static char LocalBuffer[FIXED_BUF_SIZE]; 
static unsigned char LocalBufferp; 
static char *Buffer; 

void BUF_AddChar(char c) 
{
  char CharString[2]; 
  
  sprintf(CharString, "%c", c);
  BUF_AddString(CharString);
  return;
}

void BUF_AddLong(long l)
{
  char LongString[12]; 

  sprintf(LongString, "%ld", l);
  BUF_AddString(LongString);
  return;
}

void BUF_AddBoolean(unsigned char b) 
{
  BUF_AddChar((b) ? '1' : '0');
  return;
}

void BUF_AddDouble(double d) 
{
  char str[20]; 
  int len; 
  len = (int) (((char *)sprintf(str, "%.10f", d)) - 1);

  while (*(str + len) == '0') {
    *(str + len) = '\0';
    len--;
    
    if (*(str + len) == '.') {
      *(str + len) = '\0';
      break;
    }
    
  }
  
  BUF_AddString(str);
  return;
}

  
void BUF_AddPrecisionDouble(double d) 
{
  char DoubleString[40]; 
  
  sprintf(DoubleString, "%.30e", d);
  BUF_AddString(DoubleString);
  return;
}

void BUF_AddStrings(char *string1, ...)
{
  va_list array; 
  char *bit; 
  BUF_AddString(string1);

  va_start(array, string1);

  while ((bit = va_arg(array, char *)) != NULL) {
    BUF_AddString(bit);
  }
  va_end(array);

  return;
}

void BUF_AddString(char *s) 
{
  long unsigned int Size; 

  Size = strlen(s);
  assert(s);
  if ((Index + Size + 1) > BufferSize) {
    if (LocalBufferp) {
      BufferSize <<= 1;
      Buffer = realloc(Buffer, BufferSize);
      carmen_test_alloc(Buffer);
    }
  }
  
  memcpy(Buffer + Index, s, Size+1);
  Index += Size;
  return;
}


void BUF_AddSpace() 
{
  BUF_AddString(" ");
  return;
}


char *BUF_Buffer() 
{
  return Buffer;
}

void BUF_Reset(char *Buf)
{
  Index = 0;
  if (Buf != NULL) {
    Buffer = Buf;
    LocalBufferp = 0;
  } else {
    Buffer = LocalBuffer;
    LocalBufferp = 1;
  }
  Buffer[0] = '\0';
  return;  
}

long int BUF_Index()
{
  return Index;
}

void BUF_SetIndex(long Value) 
{
  Index = Value;
  return;
}
