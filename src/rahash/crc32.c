#include "../radare.h"

static char crc_table_is_init = 0;
static unsigned int crc_table[256];

unsigned int crc32(char *buf, ut64 len)
{
	unsigned int crc = 0;

	if (!crc_table_is_init) {
		unsigned int i, j, h = 1;
		crc_table_is_init = 1;
		crc_table[0] = 0;
		for (i = 128; i; i >>= 1) {
			h = (h >> 1) ^ ((h & 1) ? (int)0xedb88320 : 0);
			for (j = 0; j < 256; j += 2*i)
			crc_table[i+j] = crc_table[j] ^ h;
		}
	}

	crc ^= 0xffffffff;
	while (len--)
	crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
	return crc ^ 0xffffffff;
}
