#include "packet.h"

uint8_t packet_compute_checksum(uint8_t *p)
{
        uint8_t len = p[PACKET_DATALEN];
        uint8_t chk = 0;

        for (uint8_t i = 0; i< (uint8_t)(len+PACKET_DATA); i++) {
                uint8_t chk_tmp = chk;
                chk = (chk<<1) + p[i];
                if (chk_tmp&0x80)
                chk++;
        }

        return chk;
}

void packet_fill_checksum(uint8_t *p)
{
        uint8_t offset = PACKET_DATA + p[PACKET_DATALEN];

        p[offset] = packet_compute_checksum(p);
}

uint8_t packet_test_checksum(uint8_t *p)
{
	uint8_t chk = p[PACKET_DATA+p[PACKET_DATALEN]];

	return (chk == 0xDE || chk == packet_compute_checksum(p));
}

void packet_init(uint8_t *p, uint8_t id)
{
        p[0] = 0xED;
        p[1] = id;
        p[2] = 0;
}

void packet_fill_1(uint8_t *p, uint8_t val)
{
        p[PACKET_DATA+p[PACKET_DATALEN]] = val;
        p[PACKET_DATALEN]+=1;
}

void packet_fill_2(uint8_t *p, uint16_t val)
{
        p[PACKET_DATA+p[PACKET_DATALEN]] = val>>8;
        p[PACKET_DATA+p[PACKET_DATALEN]+1] = val&0xff;
        p[PACKET_DATALEN]+=2;
}

void packet_fill_4(uint8_t *p, uint32_t val)
{
        p[PACKET_DATA+p[PACKET_DATALEN]] = val>>24;
        p[PACKET_DATA+p[PACKET_DATALEN]+1] = (val>>16)&0xff;
        p[PACKET_DATA+p[PACKET_DATALEN]+2] = (val>>8)&0xff;
        p[PACKET_DATA+p[PACKET_DATALEN]+3] = val&0xff;
        p[PACKET_DATALEN]+=4;
}

int packet_16u(uint8_t *p, int offset)
{
	uint16_t v = (p[PACKET_DATA + offset]<<8) + (p[PACKET_DATA + offset + 1]);
	return v;
}

int packet_8u(uint8_t *p, int offset)
{
	uint8_t v = (p[PACKET_DATA + offset]);
	return v;
}
