import re
import fileinput

instproc = re.compile(r'^inst[0-9a-f][0-9a-f]\s+proc')

with open('../mos6502/mos6502.cpp', 'r') as cpp:
    code = cpp.read()
# print(code)

with open('./m6502.asm') as asm:
  for line in asm:
    if instproc.search(line):
      instr = int(line.strip().split()[0][4:],16)
      next(asm)
      clocktime = next(asm)
      time = int(clocktime.split(';')[0].strip().split()[-1])

      replace = '	InstrTable[0x%02X] = instr;\n' % instr;
      # print('	InstrTable[%02X]' % instr)
      # print 'instr.cycles = %d;' % time
      # print(replace in code)
      # print(replace, replace+('	instr.cycles = %d;\n' % time))
      code = code.replace(replace, ('	instr.cycles = %d;\n' % time)+replace)
print(code)


with open('../mos6502/mos6502.cpp', 'w') as cpp:
    cpp.write(code)
