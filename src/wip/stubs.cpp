#include <cstdint>
#include "mouse.h"
#include "stubs.h"

#include "m6502.h"

uint8_t M_RDMEM(uint16_t) { STUB_BODY };
void M_WRMEM(uint16_t, uint8_t) { STUB_BODY };

class mouse m;

DWORD m6502clockticks = 0;
BYTE* m6502Base = NULL;

void m6502reset(void) { STUB_BODY }
DWORD m6502int(void) { STUB_BODY }
DWORD m6502nmi(void) { STUB_BODY }
DWORD m6502exec(DWORD) { STUB_BODY }

struct m6502MemoryRead *m6502MemoryRead = NULL;
struct m6502MemoryWrite *m6502MemoryWrite = NULL;

int timeperframe = 0;
