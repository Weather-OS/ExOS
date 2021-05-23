#pragma once
#include <stdint.h>

void ata_setup();
int ata_read(void* dst, uint64_t lba, uint16_t cnt);
void ata_reset();
