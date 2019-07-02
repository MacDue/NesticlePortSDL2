# NESticlePort
An experiment compiling the NESticle code released by [IamRifki](https://github.com/IamRifki/NESticle) with a current compiler.

## Status

- Some changes have been made to fix small errors (see changes.patch) e.g. code like:
  ```c++
  // Broken
  for (int i=0; i < 10; i++) { /* do something */ }

  return some_array[i]; /* I not in scope! */
  ```

- WIN95/DOS & ASM code has been stubbed out for the moment (maybe it would be possible to replace with SDL as a cross platform alternative).

- Code compiles with g++ (``gcc version 6.3.0 20170516 (Debian 6.3.0-18+deb9u1)``) [using the provided makefile].

- ~~Code seems incomplete with a lot of linker errors (missing functions) around the GUI (I think in the full source code there would be implementations of the GUI for both WIN95/DOS).~~

- Code is incomplete. The last few commits have put quite a bit of work into reverse engineering `GUI.LIB` into usable C++ code.
  - So fair `gui.cpp`, `guirect.cpp`, `guiedit.cpp`, `guimenu.cpp` have been reversed (though not tested).

Linker output:
```
obj/cpu.o:(.bss+0x1): multiple definition of `CPURunning'
obj/6502/6502.o:(.bss+0x8): first defined here
/usr/lib/gcc/x86_64-linux-gnu/6/../../../x86_64-linux-gnu/Scrt1.o: In function `_start':
(.text+0x20): undefined reference to `main'
obj/nesdlg.o: In function `DLGPOS::getnewpos(GUIbox*)':
/home/macdue/git/NESticlePort/src/dlgpos.h:21: undefined reference to `GUIdefx'
/home/macdue/git/NESticlePort/src/dlgpos.h:21: undefined reference to `GUIdefy'
/home/macdue/git/NESticlePort/src/dlgpos.h:22: undefined reference to `nextGUIdef()'
/home/macdue/git/NESticlePort/src/dlgpos.h:28: undefined reference to `resetGUIdef()'
/home/macdue/git/NESticlePort/src/dlgpos.h:29: undefined reference to `GUIdefx'
/home/macdue/git/NESticlePort/src/dlgpos.h:29: undefined reference to `GUIdefy'
obj/nesdlg.o: In function `bitmap8x8::draw_tile(char*, int, int)':
/home/macdue/git/NESticlePort/src/nesvideo.h:22: undefined reference to `draw_tile_asm'
obj/nes.o: In function `resetNEShardware()':
/home/macdue/git/NESticlePort/src/nes.cpp:86: undefined reference to `m6502Base'
/home/macdue/git/NESticlePort/src/nes.cpp:87: undefined reference to `m6502reset'
/home/macdue/git/NESticlePort/src/nes.cpp:89: undefined reference to `m6502MemoryRead'
/home/macdue/git/NESticlePort/src/nes.cpp:90: undefined reference to `m6502MemoryWrite'
obj/font.o: In function `FONT::printf(int, int, char*, ...)':
/home/macdue/git/NESticlePort/src/font.cpp:78: undefined reference to `screen'
obj/font.o: In function `IMG::draw(char*, int, int, int)':
/home/macdue/git/NESticlePort/src/r2img.h:90: undefined reference to `drawimager2'
obj/r2img.o: In function `SCR::draw(char*, int, int)':
/home/macdue/git/NESticlePort/src/r2img.cpp:79: undefined reference to `drawscr'
obj/r2img.o: In function `drawbox(char*, int, int, int, int, int)':
/home/macdue/git/NESticlePort/src/r2img.cpp:512: undefined reference to `drawhline'
/home/macdue/git/NESticlePort/src/r2img.cpp:514: undefined reference to `drawhline'
/home/macdue/git/NESticlePort/src/r2img.cpp:516: undefined reference to `drawvline'
/home/macdue/git/NESticlePort/src/r2img.cpp:518: undefined reference to `drawvline'
obj/gui.o: In function `GUIstaticimage::draw(char*)':
/home/macdue/git/NESticlePort/src/gui.cpp:79: undefined reference to `drawimager2'
obj/gui.o: In function `GUIbutton::draw(char*)':
/home/macdue/git/NESticlePort/src/gui.cpp:127: undefined reference to `drawhline'
/home/macdue/git/NESticlePort/src/gui.cpp:128: undefined reference to `drawvline'
/home/macdue/git/NESticlePort/src/gui.cpp:129: undefined reference to `drawvline'
/home/macdue/git/NESticlePort/src/gui.cpp:130: undefined reference to `drawhline'
/home/macdue/git/NESticlePort/src/gui.cpp:133: undefined reference to `drawhline'
/home/macdue/git/NESticlePort/src/gui.cpp:134: undefined reference to `drawvline'
/home/macdue/git/NESticlePort/src/gui.cpp:135: undefined reference to `drawvline'
/home/macdue/git/NESticlePort/src/gui.cpp:136: undefined reference to `drawhline'
/home/macdue/git/NESticlePort/src/gui.cpp:141: undefined reference to `m'
/home/macdue/git/NESticlePort/src/gui.cpp:141: undefined reference to `m'
obj/gui.o: In function `GUIimagebutton::draw(char*)':
/home/macdue/git/NESticlePort/src/gui.cpp:193: undefined reference to `drawimager2'
obj/gui.o: In function `GUIcheckbox::draw(char*)':
/home/macdue/git/NESticlePort/src/gui.cpp:260: undefined reference to `drawimager2'
obj/gui.o: In function `GUIlistbox::draw(char*)':
/home/macdue/git/NESticlePort/src/gui.cpp:937: undefined reference to `m'
obj/main.o: In function `calcfps()':
/home/macdue/git/NESticlePort/src/main.cpp:196: undefined reference to `timeperframe'
/home/macdue/git/NESticlePort/src/main.cpp:198: undefined reference to `timeperframe'
obj/main.o: In function `updatescreen()':
/home/macdue/git/NESticlePort/src/main.cpp:219: undefined reference to `screen'
/home/macdue/git/NESticlePort/src/main.cpp:220: undefined reference to `screen'
/home/macdue/git/NESticlePort/src/main.cpp:220: undefined reference to `m'
/home/macdue/git/NESticlePort/src/main.cpp:221: undefined reference to `screen'
obj/guimenu.o: In function `menuitem::draw(int, int, int, int)':
/home/macdue/git/NESticlePort/src/guimenu.cpp:17: undefined reference to `screen'
/home/macdue/git/NESticlePort/src/guimenu.cpp:19: undefined reference to `screen'
obj/guimenu.o: In function `GUIvmenu::draw(char*)':
/home/macdue/git/NESticlePort/src/guimenu.cpp:248: undefined reference to `screen'
/home/macdue/git/NESticlePort/src/guimenu.cpp:250: undefined reference to `screen'
obj/guimenu.o:/home/macdue/git/NESticlePort/src/guimenu.cpp:253: more undefined references to `screen' follow
obj/guimenu.o: In function `GUIpopupmenu::GUIpopupmenu(GUIrect*, menu*, int, int)':
/home/macdue/git/NESticlePort/src/guimenu.cpp:328: undefined reference to `vtable for GUIpopupmenu'
obj/guimenu.o:(.data.rel.ro._ZTV8GUIhmenu[_ZTV8GUIhmenu]+0x58): undefined reference to `GUImenu::losechildfocus()'
obj/guimenu.o:(.data.rel.ro._ZTV8GUIhmenu[_ZTV8GUIhmenu]+0x80): undefined reference to `GUIhmenu::keyhit(char, char)'
obj/guimenu.o:(.data.rel.ro._ZTV8GUIvmenu[_ZTV8GUIvmenu]+0x58): undefined reference to `GUImenu::losechildfocus()'
obj/guimenu.o:(.data.rel.ro._ZTV7GUImenu[_ZTV7GUImenu]+0x58): undefined reference to `GUImenu::losechildfocus()'
obj/guirect.o: In function `GUIrect::setmodal(GUIrect*)':
/home/macdue/git/NESticlePort/src/guirect.cpp:21: undefined reference to `m'
obj/guirect.o: In function `GUIrect::fill(char)':
/home/macdue/git/NESticlePort/src/guirect.cpp:294: undefined reference to `screen'
obj/guirect.o: In function `GUIrect::outline(char)':
/home/macdue/git/NESticlePort/src/guirect.cpp:300: undefined reference to `screen'
obj/message.o: In function `MESSAGE::draw(int, int)':
/home/macdue/git/NESticlePort/src/message.cpp:15: undefined reference to `screen'
obj/6502/6502.o: In function `Reset6502()':
```
