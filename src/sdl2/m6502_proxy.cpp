#include <cstdint>

#include "m6502/m6502.h"
#include "mos6502/mos6502.h"


DWORD m6502clockticks = 0;
struct m6502MemoryRead *m6502MemoryRead = NULL;
struct m6502MemoryWrite *m6502MemoryWrite = NULL;


uint8_t MemoryRead(uint16_t address) {
  if (!m6502Base) {
    return -1;
  }
  for (
    struct m6502MemoryRead *memRead = m6502MemoryRead;
    m6502MemoryRead && memRead->memoryCall;
    memRead++
  ) {
    if (address >= memRead->lowAddr && address <= memRead->highAddr) {
      return memRead->memoryCall(address);
    }
  }
  return m6502Base[address];
}


void MemoryWrite(uint16_t address, uint8_t value) {
  if (!m6502Base) {
    return;
  }
  for (
    struct m6502MemoryWrite *memWrite = m6502MemoryWrite;
    m6502MemoryWrite && memWrite->memoryCall;
    memWrite++
  ) {
    if (address >= memWrite->lowAddr && address <= memWrite->highAddr) {
      /* TODO: Do something with return */
      memWrite->memoryCall(address, value);
      return;
    }
  }
  m6502Base[address] = value;
}


mos6502 mos6502(MemoryRead, MemoryWrite);


void m6502reset(void) {
  mos6502.Reset();
}


DWORD m6502nmi(void) {
  mos6502.NMI();
}


DWORD m6502exec(DWORD n) {
  m6502clockticks += 4 * n; // approximate for test
  mos6502.Run(n);
}
