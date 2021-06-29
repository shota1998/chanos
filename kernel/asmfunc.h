#pragma once

#include <stdint.h>

extern "C" {
	void IoOut32(uint16_t addr, uint32_t data);
	uint32_t IoInt32(uint16_t addr);
}