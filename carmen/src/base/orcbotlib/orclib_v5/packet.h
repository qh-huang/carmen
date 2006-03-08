#ifndef _PACKET_H
#define _PACKET_H

#include <string.h>
#include <stdint.h>

#include "orcconstants.h"

uint8_t packet_compute_checksum(uint8_t *p);
void packet_fill_checksum(uint8_t *p);
uint8_t packet_test_checksum(uint8_t *p);

void packet_init(uint8_t *p, uint8_t id);
void packet_fill_1(uint8_t *p, uint8_t val);
void packet_fill_2(uint8_t *p, uint16_t val);
void packet_fill_4(uint8_t *p, uint32_t val);

int packet_16u(uint8_t *p, int offset);
int packet_8u(uint8_t *p, int offset);

#endif

