#ifndef UTILS_H
#define UTILS_H

#include "stdint.h"
#include <QString>

void hexdump(const uint8_t *buf, uint32_t len);
void dump_flash_code(const uint32_t *buf, uint32_t len);

QString convert_bytes_speed_unit(uint64_t bytes);

#endif // UTILS_H
