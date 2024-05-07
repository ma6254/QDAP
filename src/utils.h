#ifndef UTILS_H
#define UTILS_H

#include "stdint.h"

void hexdump(const uint8_t *buf, uint32_t len);
void dump_flash_code(const uint32_t *buf, uint32_t len);

#endif // UTILS_H
