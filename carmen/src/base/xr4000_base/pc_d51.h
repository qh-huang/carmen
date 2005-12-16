#ifndef PC_D51_H
#define PC_D51_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned char d51_read_packet(unsigned char wait_flag, 
			      unsigned char *data_ptr);
unsigned char d51_write_packet(unsigned char *data_ptr);
unsigned char d51_init();
void d51_exit();
void d51_check_int();

void DELAYMS(unsigned int ms);

void DELAY10US(unsigned int us);

#ifdef __cplusplus
extern "C" {
#endif

#endif
