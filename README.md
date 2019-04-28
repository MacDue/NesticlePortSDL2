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

- Code seems incomplete with a lot of linker errors (missing functions) around the GUI (I think in the full source code there would be implementations of the GUI for both WIN95/DOS).

Linker output:
```
obj/cpu.o:(.bss+0x1): multiple definition of `CPURunning'
obj/6502/6502.o:(.bss+0x8): first defined here
/usr/lib/gcc/x86_64-linux-gnu/6/../../../x86_64-linux-gnu/Scrt1.o: In function `_start':
(.text+0x20): undefined reference to `main'
obj/nesdlg.o: In function `filldirlistbox(char*, GUIdirlistbox*)':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:51: undefined reference to `GUIlistbox::resizeitems(int)'
obj/nesdlg.o: In function `m_load()':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:92: undefined reference to `GUIonebuttonbox::GUIonebuttonbox(GUIrect*, char*, GUIcontents*, char*, int, int)'
obj/nesdlg.o: In function `m_patternview()':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:156: undefined reference to `GUIbox::GUIbox(GUIrect*, char*, GUIcontents*, int, int)'
obj/nesdlg.o: In function `m_nametableview()':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:199: undefined reference to `GUIbox::GUIbox(GUIrect*, char*, GUIcontents*, int, int)'
obj/nesdlg.o: In function `m_paletteview()':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:242: undefined reference to `GUIbox::GUIbox(GUIrect*, char*, GUIcontents*, int, int)'
obj/nesdlg.o: In function `GUIcontents::GUIcontents(int, int)':
/home/macdue/git/NESticlePort/src/guirect.h:79: undefined reference to `GUIrect::GUIrect(GUIrect*, int, int, int, int)'
/home/macdue/git/NESticlePort/src/guirect.h:79: undefined reference to `vtable for GUIcontents'
obj/nesdlg.o: In function `GUIlistbox::~GUIlistbox()':
/home/macdue/git/NESticlePort/src/gui.h:530: undefined reference to `vtable for GUIlistbox'
/home/macdue/git/NESticlePort/src/gui.h:530: undefined reference to `GUIlistbox::freeitems()'
/home/macdue/git/NESticlePort/src/gui.h:530: undefined reference to `GUIrect::~GUIrect()'
obj/nesdlg.o: In function `GUIstringlistbox::GUIstringlistbox(GUIrect*, int, int, int, int, int)':
/home/macdue/git/NESticlePort/src/gui.h:549: undefined reference to `GUIlistbox::GUIlistbox(GUIrect*, int, int, int, int, int)'
/home/macdue/git/NESticlePort/src/gui.h:549: undefined reference to `vtable for GUIstringlistbox'
obj/nesdlg.o: In function `DLGPOS::getnewpos(GUIbox*)':
/home/macdue/git/NESticlePort/src/dlgpos.h:21: undefined reference to `GUIdefx'
/home/macdue/git/NESticlePort/src/dlgpos.h:21: undefined reference to `GUIdefy'
/home/macdue/git/NESticlePort/src/dlgpos.h:22: undefined reference to `nextGUIdef()'
/home/macdue/git/NESticlePort/src/dlgpos.h:28: undefined reference to `resetGUIdef()'
/home/macdue/git/NESticlePort/src/dlgpos.h:29: undefined reference to `GUIdefx'
/home/macdue/git/NESticlePort/src/dlgpos.h:29: undefined reference to `GUIdefy'
obj/nesdlg.o: In function `DLGPOS::open(GUIbox*)':
/home/macdue/git/NESticlePort/src/dlgpos.h:42: undefined reference to `GUIrect::setfocus(GUIrect*)'
/home/macdue/git/NESticlePort/src/dlgpos.h:44: undefined reference to `GUIrect::moverel(int, int)'
obj/nesdlg.o: In function `bitmap8x8::draw_tile(char*, int, int)':
/home/macdue/git/NESticlePort/src/nesvideo.h:22: undefined reference to `draw_tile_asm'
obj/nesdlg.o: In function `GUIstringlistbox::~GUIstringlistbox()':
/home/macdue/git/NESticlePort/src/gui.h:543: undefined reference to `vtable for GUIstringlistbox'
obj/nesdlg.o: In function `GUIcontents::~GUIcontents()':
/home/macdue/git/NESticlePort/src/guirect.h:76: undefined reference to `vtable for GUIcontents'
/home/macdue/git/NESticlePort/src/guirect.h:76: undefined reference to `GUIrect::~GUIrect()'
obj/nesdlg.o: In function `loaddlg::draw(char*)':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:82: undefined reference to `GUIrect::fill(char)'
/home/macdue/git/NESticlePort/src/nesdlg.cpp:82: undefined reference to `GUIrect::draw(char*)'
obj/nesdlg.o: In function `patterntableview::patterntableview(GUIrect*, int)':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:111: undefined reference to `GUIrect::GUIrect(GUIrect*, int, int, int, int)'
obj/nesdlg.o: In function `patterndlg::patterndlg()':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:138: undefined reference to `GUIrect::moveto(int, int)'
obj/nesdlg.o:(.data.rel.ro._ZTV14paletteviewdlg[_ZTV14paletteviewdlg]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/nesdlg.o:(.data.rel.ro._ZTV14paletteviewdlg[_ZTV14paletteviewdlg]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/nesdlg.o:(.data.rel.ro._ZTV14paletteviewdlg[_ZTV14paletteviewdlg]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/nesdlg.o:(.data.rel.ro._ZTV14paletteviewdlg[_ZTV14paletteviewdlg]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV14paletteviewdlg[_ZTV14paletteviewdlg]+0x50): undefined reference to `GUIrect::losefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV14paletteviewdlg[_ZTV14paletteviewdlg]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV14paletteviewdlg[_ZTV14paletteviewdlg]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/nesdlg.o:(.data.rel.ro._ZTV7namedlg[_ZTV7namedlg]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/nesdlg.o:(.data.rel.ro._ZTV7namedlg[_ZTV7namedlg]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/nesdlg.o:(.data.rel.ro._ZTV7namedlg[_ZTV7namedlg]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/nesdlg.o:(.data.rel.ro._ZTV7namedlg[_ZTV7namedlg]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV7namedlg[_ZTV7namedlg]+0x50): undefined reference to `GUIrect::losefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV7namedlg[_ZTV7namedlg]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV7namedlg[_ZTV7namedlg]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/nesdlg.o:(.data.rel.ro._ZTV10patterndlg[_ZTV10patterndlg]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/nesdlg.o:(.data.rel.ro._ZTV10patterndlg[_ZTV10patterndlg]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/nesdlg.o:(.data.rel.ro._ZTV10patterndlg[_ZTV10patterndlg]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/nesdlg.o:(.data.rel.ro._ZTV10patterndlg[_ZTV10patterndlg]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV10patterndlg[_ZTV10patterndlg]+0x50): undefined reference to `GUIrect::losefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV10patterndlg[_ZTV10patterndlg]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV10patterndlg[_ZTV10patterndlg]+0x78): undefined reference to `GUIrect::draw(char*)'
obj/nesdlg.o:(.data.rel.ro._ZTV10patterndlg[_ZTV10patterndlg]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/nesdlg.o:(.data.rel.ro._ZTV16patterntableview[_ZTV16patterntableview]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/nesdlg.o:(.data.rel.ro._ZTV16patterntableview[_ZTV16patterntableview]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/nesdlg.o:(.data.rel.ro._ZTV16patterntableview[_ZTV16patterntableview]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/nesdlg.o:(.data.rel.ro._ZTV16patterntableview[_ZTV16patterntableview]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV16patterntableview[_ZTV16patterntableview]+0x50): undefined reference to `GUIrect::losefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV16patterntableview[_ZTV16patterntableview]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV16patterntableview[_ZTV16patterntableview]+0x80): undefined reference to `GUIrect::keyhit(char, char)'
obj/nesdlg.o: In function `patterntableview::~patterntableview()':
/home/macdue/git/NESticlePort/src/nesdlg.cpp:104: undefined reference to `GUIrect::~GUIrect()'
obj/nesdlg.o:(.data.rel.ro._ZTV7loaddlg[_ZTV7loaddlg]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/nesdlg.o:(.data.rel.ro._ZTV7loaddlg[_ZTV7loaddlg]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/nesdlg.o:(.data.rel.ro._ZTV7loaddlg[_ZTV7loaddlg]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/nesdlg.o:(.data.rel.ro._ZTV7loaddlg[_ZTV7loaddlg]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV7loaddlg[_ZTV7loaddlg]+0x50): undefined reference to `GUIrect::losefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV7loaddlg[_ZTV7loaddlg]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV7loaddlg[_ZTV7loaddlg]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x50): undefined reference to `GUIrect::losefocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x60): undefined reference to `GUIlistbox::click(mouse&)'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x68): undefined reference to `GUIlistbox::release(mouse&)'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x70): undefined reference to `GUIlistbox::drag(mouse&)'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x78): undefined reference to `GUIlistbox::draw(char*)'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x80): undefined reference to `GUIlistbox::keyhit(char, char)'
obj/nesdlg.o:(.data.rel.ro._ZTV13GUIdirlistbox[_ZTV13GUIdirlistbox]+0x98): undefined reference to `GUIstringlistbox::drawitems(char*, int, int)'
obj/nesdlg.o:(.data.rel.ro._ZTI14paletteviewdlg[_ZTI14paletteviewdlg]+0x10): undefined reference to `typeinfo for GUIcontents'
obj/nesdlg.o:(.data.rel.ro._ZTI7namedlg[_ZTI7namedlg]+0x10): undefined reference to `typeinfo for GUIcontents'
obj/nesdlg.o:(.data.rel.ro._ZTI10patterndlg[_ZTI10patterndlg]+0x10): undefined reference to `typeinfo for GUIcontents'
obj/nesdlg.o:(.data.rel.ro._ZTI16patterntableview[_ZTI16patterntableview]+0x10): undefined reference to `typeinfo for GUIrect'
obj/nesdlg.o:(.data.rel.ro._ZTI7loaddlg[_ZTI7loaddlg]+0x10): undefined reference to `typeinfo for GUIcontents'
obj/nesdlg.o:(.data.rel.ro._ZTI13GUIdirlistbox[_ZTI13GUIdirlistbox]+0x10): undefined reference to `typeinfo for GUIstringlistbox'
obj/nes.o: In function `resetNEShardware()':
/home/macdue/git/NESticlePort/src/nes.cpp:86: undefined reference to `m6502Base'
/home/macdue/git/NESticlePort/src/nes.cpp:87: undefined reference to `m6502reset'
/home/macdue/git/NESticlePort/src/nes.cpp:89: undefined reference to `m6502MemoryRead'
/home/macdue/git/NESticlePort/src/nes.cpp:90: undefined reference to `m6502MemoryWrite'
obj/stddlg.o: In function `setres(int, int)':
/home/macdue/git/NESticlePort/src/stddlg.cpp:148: undefined reference to `GUImaximizebox::maximize()'
obj/stddlg.o: In function `disablegui()':
/home/macdue/git/NESticlePort/src/stddlg.cpp:216: undefined reference to `GUIrect::setfocus(GUIrect*)'
obj/stddlg.o: In function `initdefaultgui()':
/home/macdue/git/NESticlePort/src/stddlg.cpp:354: undefined reference to `GUIhmenu::GUIhmenu(GUIrect*, menu*, int, int)'
obj/stddlg.o: In function `GUImessage::setrange()':
/home/macdue/git/NESticlePort/src/stddlg.cpp:39: undefined reference to `GUIscrollbar::setrange(int, int)'
/home/macdue/git/NESticlePort/src/stddlg.cpp:40: undefined reference to `GUIscrollbar::setpos(int)'
obj/stddlg.o: In function `GUImessage::GUImessage(msgbuffer*)':
/home/macdue/git/NESticlePort/src/stddlg.cpp:47: undefined reference to `GUIvscrollbar::GUIvscrollbar(GUIrect*, int, int, int)'
obj/stddlg.o: In function `GUImessage::draw(char*)':
/home/macdue/git/NESticlePort/src/stddlg.cpp:54: undefined reference to `GUIrect::fill(char)'
/home/macdue/git/NESticlePort/src/stddlg.cpp:56: undefined reference to `GUIrect::draw(char*)'
obj/stddlg.o: In function `systemmessages::open()':
/home/macdue/git/NESticlePort/src/stddlg.cpp:74: undefined reference to `GUIbox::GUIbox(GUIrect*, char*, GUIcontents*, int, int)'
obj/stddlg.o: In function `aboutdlg::aboutdlg()':
/home/macdue/git/NESticlePort/src/stddlg.cpp:95: undefined reference to `GUIstaticimage::GUIstaticimage(GUIrect*, IMG*, int, int)'
/home/macdue/git/NESticlePort/src/stddlg.cpp:96: undefined reference to `GUIstatictext::GUIstatictext(GUIrect*, int, char*, int, int)'
/home/macdue/git/NESticlePort/src/stddlg.cpp:98: undefined reference to `GUIstatictext::GUIstatictext(GUIrect*, int, char*, int, int)'
/home/macdue/git/NESticlePort/src/stddlg.cpp:105: undefined reference to `GUIstatictext::GUIstatictext(GUIrect*, int, char*, int, int)'
/home/macdue/git/NESticlePort/src/stddlg.cpp:107: undefined reference to `GUIstatictext::GUIstatictext(GUIrect*, int, char*, int, int)'
/home/macdue/git/NESticlePort/src/stddlg.cpp:108: undefined reference to `buildcompilerversionlow'
/home/macdue/git/NESticlePort/src/stddlg.cpp:108: undefined reference to `buildcompilerversionhigh'
/home/macdue/git/NESticlePort/src/stddlg.cpp:108: undefined reference to `buildcompiler'
/home/macdue/git/NESticlePort/src/stddlg.cpp:109: undefined reference to `GUIstatictext::GUIstatictext(GUIrect*, int, char*, int, int)'
obj/stddlg.o: In function `aboutdlg::draw(char*)':
/home/macdue/git/NESticlePort/src/stddlg.cpp:115: undefined reference to `GUIrect::fill(char)'
/home/macdue/git/NESticlePort/src/stddlg.cpp:115: undefined reference to `GUIrect::draw(char*)'
obj/stddlg.o: In function `aboutdlg::open()':
/home/macdue/git/NESticlePort/src/stddlg.cpp:121: undefined reference to `GUIonebuttonbox::GUIonebuttonbox(GUIrect*, char*, GUIcontents*, char*, int, int)'
obj/stddlg.o:(.data.rel.ro._ZTV8aboutdlg[_ZTV8aboutdlg]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/stddlg.o:(.data.rel.ro._ZTV8aboutdlg[_ZTV8aboutdlg]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/stddlg.o:(.data.rel.ro._ZTV8aboutdlg[_ZTV8aboutdlg]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/stddlg.o:(.data.rel.ro._ZTV8aboutdlg[_ZTV8aboutdlg]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/stddlg.o:(.data.rel.ro._ZTV8aboutdlg[_ZTV8aboutdlg]+0x50): undefined reference to `GUIrect::losefocus()'
obj/stddlg.o:(.data.rel.ro._ZTV8aboutdlg[_ZTV8aboutdlg]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/stddlg.o:(.data.rel.ro._ZTV8aboutdlg[_ZTV8aboutdlg]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/stddlg.o:(.data.rel.ro._ZTV14systemmessages[_ZTV14systemmessages]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/stddlg.o:(.data.rel.ro._ZTV14systemmessages[_ZTV14systemmessages]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/stddlg.o:(.data.rel.ro._ZTV14systemmessages[_ZTV14systemmessages]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/stddlg.o:(.data.rel.ro._ZTV14systemmessages[_ZTV14systemmessages]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/stddlg.o:(.data.rel.ro._ZTV14systemmessages[_ZTV14systemmessages]+0x50): undefined reference to `GUIrect::losefocus()'
obj/stddlg.o:(.data.rel.ro._ZTV14systemmessages[_ZTV14systemmessages]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/stddlg.o:(.data.rel.ro._ZTV14systemmessages[_ZTV14systemmessages]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/stddlg.o:(.data.rel.ro._ZTV10GUImessage[_ZTV10GUImessage]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/stddlg.o:(.data.rel.ro._ZTV10GUImessage[_ZTV10GUImessage]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/stddlg.o:(.data.rel.ro._ZTV10GUImessage[_ZTV10GUImessage]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/stddlg.o:(.data.rel.ro._ZTV10GUImessage[_ZTV10GUImessage]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/stddlg.o:(.data.rel.ro._ZTV10GUImessage[_ZTV10GUImessage]+0x50): undefined reference to `GUIrect::losefocus()'
obj/stddlg.o:(.data.rel.ro._ZTV10GUImessage[_ZTV10GUImessage]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/stddlg.o:(.data.rel.ro._ZTV10GUImessage[_ZTV10GUImessage]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/stddlg.o:(.data.rel.ro._ZTI8aboutdlg[_ZTI8aboutdlg]+0x10): undefined reference to `typeinfo for GUIcontents'
obj/stddlg.o:(.data.rel.ro._ZTI10GUImessage[_ZTI10GUImessage]+0x10): undefined reference to `typeinfo for GUIcontents'
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
obj/mouse.o: In function `mouse::updatebut(int)':
/home/macdue/git/NESticlePort/src/mouse.cpp:43: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/mouse.cpp:45: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/mouse.cpp:45: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/mouse.cpp:63: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/mouse.cpp:63: undefined reference to `GUIrect::modal'
obj/main.o: In function `initgame()':
/home/macdue/git/NESticlePort/src/main.cpp:144: undefined reference to `GUIroot::GUIroot(ROOT*)'
obj/main.o: In function `calcfps()':
/home/macdue/git/NESticlePort/src/main.cpp:196: undefined reference to `timeperframe'
/home/macdue/git/NESticlePort/src/main.cpp:198: undefined reference to `timeperframe'
obj/main.o: In function `updatescreen()':
/home/macdue/git/NESticlePort/src/main.cpp:219: undefined reference to `screen'
/home/macdue/git/NESticlePort/src/main.cpp:220: undefined reference to `screen'
/home/macdue/git/NESticlePort/src/main.cpp:220: undefined reference to `m'
/home/macdue/git/NESticlePort/src/main.cpp:221: undefined reference to `screen'
obj/main.o: In function `ROOT::ROOT()':
/home/macdue/git/NESticlePort/src/main.cpp:252: undefined reference to `GUIrect::GUIrect(GUIrect*, int, int, int, int)'
obj/main.o: In function `ROOT::~ROOT()':
/home/macdue/git/NESticlePort/src/main.cpp:253: undefined reference to `GUIrect::~GUIrect()'
obj/main.o: In function `ROOT::keyhit(char, char)':
/home/macdue/git/NESticlePort/src/guiroot.h:17: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/guiroot.h:17: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/guiroot.h:17: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/guiroot.h:18: undefined reference to `GUIrect::keyhit(char, char)'
obj/main.o: In function `ROOT::hittest(int, int)':
/home/macdue/git/NESticlePort/src/guiroot.h:22: undefined reference to `GUIrect::hittest(int, int)'
/home/macdue/git/NESticlePort/src/guiroot.h:23: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/guiroot.h:23: undefined reference to `GUIrect::setfocus(GUIrect*)'
obj/main.o:(.data.rel.ro._ZTV4ROOT[_ZTV4ROOT]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/main.o:(.data.rel.ro._ZTV4ROOT[_ZTV4ROOT]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/main.o:(.data.rel.ro._ZTV4ROOT[_ZTV4ROOT]+0x78): undefined reference to `GUIrect::draw(char*)'
obj/main.o:(.data.rel.ro._ZTI4ROOT[_ZTI4ROOT]+0x10): undefined reference to `typeinfo for GUIrect'
obj/inputw.o: In function `GUIedit::~GUIedit()':
/home/macdue/git/NESticlePort/src/gui.h:151: undefined reference to `vtable for GUIedit'
/home/macdue/git/NESticlePort/src/gui.h:151: undefined reference to `GUIrect::~GUIrect()'
obj/inputw.o: In function `keyedit::keyedit(GUIrect*, char*, int, int, char&)':
/home/macdue/git/NESticlePort/src/inputw.cpp:44: undefined reference to `GUIedit::GUIedit(GUIrect*, char*, int, int, int)'
/home/macdue/git/NESticlePort/src/inputw.cpp:46: undefined reference to `GUIrect::moverel(int, int)'
obj/inputw.o: In function `keyedit::click(mouse&)':
/home/macdue/git/NESticlePort/src/inputw.cpp:52: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/inputw.cpp:52: undefined reference to `GUIrect::setmodal(GUIrect*)'
/home/macdue/git/NESticlePort/src/inputw.cpp:52: undefined reference to `GUIrect::setmodal(GUIrect*)'
/home/macdue/git/NESticlePort/src/inputw.cpp:53: undefined reference to `GUIrect::setfocus(GUIrect*)'
obj/inputw.o: In function `keyedit::keyhit(char, char)':
/home/macdue/git/NESticlePort/src/inputw.cpp:61: undefined reference to `GUIrect::modal'
/home/macdue/git/NESticlePort/src/inputw.cpp:63: undefined reference to `GUIrect::setmodal(GUIrect*)'
/home/macdue/git/NESticlePort/src/inputw.cpp:64: undefined reference to `GUIrect::setmodal(GUIrect*)'
/home/macdue/git/NESticlePort/src/inputw.cpp:71: undefined reference to `GUIrect::cyclefocus(int)'
/home/macdue/git/NESticlePort/src/inputw.cpp:73: undefined reference to `GUIrect::setmodal(GUIrect*)'
obj/inputw.o: In function `keyedit::drawdata(char*, int, int, int)':
/home/macdue/git/NESticlePort/src/inputw.cpp:79: undefined reference to `GUIrect::modal'
obj/inputw.o: In function `redefinekeys::~redefinekeys()':
/home/macdue/git/NESticlePort/src/inputw.cpp:121: undefined reference to `GUIrect::losefocus()'
obj/inputw.o: In function `redefinekeys::draw(char*)':
/home/macdue/git/NESticlePort/src/inputw.cpp:128: undefined reference to `GUIrect::fill(char)'
/home/macdue/git/NESticlePort/src/inputw.cpp:129: undefined reference to `GUIrect::draw(char*)'
obj/inputw.o: In function `redefinekeys::open(keymap*)':
/home/macdue/git/NESticlePort/src/inputw.cpp:138: undefined reference to `GUIbox::GUIbox(GUIrect*, char*, GUIcontents*, int, int)'
obj/inputw.o: In function `inputtestbutton::inputtestbutton(GUIrect*, int, int, int, int, input*, int)':
/home/macdue/git/NESticlePort/src/inputw.cpp:159: undefined reference to `GUIrect::GUIrect(GUIrect*, int, int, int, int)'
obj/inputw.o: In function `inputtestbutton::draw(char*)':
/home/macdue/git/NESticlePort/src/inputw.cpp:165: undefined reference to `GUIrect::fill(char)'
/home/macdue/git/NESticlePort/src/inputw.cpp:166: undefined reference to `GUIrect::fill(char)'
/home/macdue/git/NESticlePort/src/inputw.cpp:167: undefined reference to `GUIrect::outline(char)'
obj/inputw.o: In function `inputtestarea::inputtestarea(GUIrect*, int, int, int, input*)':
/home/macdue/git/NESticlePort/src/inputw.cpp:178: undefined reference to `GUIrect::GUIrect(GUIrect*, int, int, int, int)'
obj/inputw.o: In function `inputtestarea::draw(char*)':
/home/macdue/git/NESticlePort/src/inputw.cpp:186: undefined reference to `GUIrect::fill(char)'
/home/macdue/git/NESticlePort/src/inputw.cpp:187: undefined reference to `GUIrect::outline(char)'
obj/inputw.o: In function `changeinputdevice::changeinputdevice(int, input*&)':
/home/macdue/git/NESticlePort/src/inputw.cpp:222: undefined reference to `GUIstatictext::GUIstatictext(GUIrect*, int, char*, int, int)'
/home/macdue/git/NESticlePort/src/inputw.cpp:226: undefined reference to `GUIlistbox::resizeitems(int)'
/home/macdue/git/NESticlePort/src/inputw.cpp:229: undefined reference to `GUIstatictext::GUIstatictext(GUIrect*, int, char*, int, int)'
/home/macdue/git/NESticlePort/src/inputw.cpp:237: undefined reference to `GUIlistbox::setsel(int)'
obj/inputw.o: In function `changeinputdevice::losefocus()':
/home/macdue/git/NESticlePort/src/inputw.cpp:266: undefined reference to `GUIrect::losefocus()'
obj/inputw.o: In function `changeinputdevice::receivefocus()':
/home/macdue/git/NESticlePort/src/inputw.cpp:272: undefined reference to `GUIrect::receivefocus()'
obj/inputw.o: In function `changeinputdevice::sendmessage(GUIrect*, int)':
/home/macdue/git/NESticlePort/src/inputw.cpp:319: undefined reference to `GUItextbutton::GUItextbutton(GUIrect*, char*, int, int)'
/home/macdue/git/NESticlePort/src/inputw.cpp:320: undefined reference to `GUItextbutton::GUItextbutton(GUIrect*, char*, int, int)'
/home/macdue/git/NESticlePort/src/inputw.cpp:323: undefined reference to `GUItextbutton::GUItextbutton(GUIrect*, char*, int, int)'
obj/inputw.o: In function `changeinputdevice::draw(char*)':
/home/macdue/git/NESticlePort/src/inputw.cpp:332: undefined reference to `GUIrect::fill(char)'
/home/macdue/git/NESticlePort/src/inputw.cpp:340: undefined reference to `GUIrect::draw(char*)'
obj/inputw.o: In function `changeinputdevice::open(int)':
/home/macdue/git/NESticlePort/src/inputw.cpp:350: undefined reference to `GUIbox::GUIbox(GUIrect*, char*, GUIcontents*, int, int)'
obj/inputw.o:(.data.rel.ro._ZTV17changeinputdevice[_ZTV17changeinputdevice]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/inputw.o:(.data.rel.ro._ZTV17changeinputdevice[_ZTV17changeinputdevice]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/inputw.o:(.data.rel.ro._ZTV17changeinputdevice[_ZTV17changeinputdevice]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/inputw.o:(.data.rel.ro._ZTV17changeinputdevice[_ZTV17changeinputdevice]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/inputw.o:(.data.rel.ro._ZTV17changeinputdevice[_ZTV17changeinputdevice]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/inputw.o:(.data.rel.ro._ZTV13inputtestarea[_ZTV13inputtestarea]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/inputw.o:(.data.rel.ro._ZTV13inputtestarea[_ZTV13inputtestarea]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/inputw.o:(.data.rel.ro._ZTV13inputtestarea[_ZTV13inputtestarea]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/inputw.o:(.data.rel.ro._ZTV13inputtestarea[_ZTV13inputtestarea]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/inputw.o:(.data.rel.ro._ZTV13inputtestarea[_ZTV13inputtestarea]+0x50): undefined reference to `GUIrect::losefocus()'
obj/inputw.o:(.data.rel.ro._ZTV13inputtestarea[_ZTV13inputtestarea]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/inputw.o:(.data.rel.ro._ZTV13inputtestarea[_ZTV13inputtestarea]+0x80): undefined reference to `GUIrect::keyhit(char, char)'
obj/inputw.o: In function `inputtestarea::~inputtestarea()':
/home/macdue/git/NESticlePort/src/inputw.cpp:172: undefined reference to `GUIrect::~GUIrect()'
obj/inputw.o:(.data.rel.ro._ZTV15inputtestbutton[_ZTV15inputtestbutton]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/inputw.o:(.data.rel.ro._ZTV15inputtestbutton[_ZTV15inputtestbutton]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/inputw.o:(.data.rel.ro._ZTV15inputtestbutton[_ZTV15inputtestbutton]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/inputw.o:(.data.rel.ro._ZTV15inputtestbutton[_ZTV15inputtestbutton]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/inputw.o:(.data.rel.ro._ZTV15inputtestbutton[_ZTV15inputtestbutton]+0x50): undefined reference to `GUIrect::losefocus()'
obj/inputw.o:(.data.rel.ro._ZTV15inputtestbutton[_ZTV15inputtestbutton]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/inputw.o:(.data.rel.ro._ZTV15inputtestbutton[_ZTV15inputtestbutton]+0x80): undefined reference to `GUIrect::keyhit(char, char)'
obj/inputw.o: In function `inputtestbutton::~inputtestbutton()':
/home/macdue/git/NESticlePort/src/inputw.cpp:149: undefined reference to `GUIrect::~GUIrect()'
obj/inputw.o:(.data.rel.ro._ZTV12redefinekeys[_ZTV12redefinekeys]+0x20): undefined reference to `GUIrect::hittest(int, int)'
obj/inputw.o:(.data.rel.ro._ZTV12redefinekeys[_ZTV12redefinekeys]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/inputw.o:(.data.rel.ro._ZTV12redefinekeys[_ZTV12redefinekeys]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/inputw.o:(.data.rel.ro._ZTV12redefinekeys[_ZTV12redefinekeys]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/inputw.o:(.data.rel.ro._ZTV12redefinekeys[_ZTV12redefinekeys]+0x50): undefined reference to `GUIrect::losefocus()'
obj/inputw.o:(.data.rel.ro._ZTV12redefinekeys[_ZTV12redefinekeys]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/inputw.o:(.data.rel.ro._ZTV12redefinekeys[_ZTV12redefinekeys]+0x80): undefined reference to `GUIcontents::keyhit(char, char)'
obj/inputw.o:(.data.rel.ro._ZTV7keyedit[_ZTV7keyedit]+0x20): undefined reference to `GUIedit::hittest(int, int)'
obj/inputw.o:(.data.rel.ro._ZTV7keyedit[_ZTV7keyedit]+0x28): undefined reference to `GUIrect::bringtofront()'
obj/inputw.o:(.data.rel.ro._ZTV7keyedit[_ZTV7keyedit]+0x30): undefined reference to `GUIrect::sendtoback()'
obj/inputw.o:(.data.rel.ro._ZTV7keyedit[_ZTV7keyedit]+0x48): undefined reference to `GUIrect::receivefocus()'
obj/inputw.o:(.data.rel.ro._ZTV7keyedit[_ZTV7keyedit]+0x50): undefined reference to `GUIrect::losefocus()'
obj/inputw.o:(.data.rel.ro._ZTV7keyedit[_ZTV7keyedit]+0x58): undefined reference to `GUIrect::losechildfocus()'
obj/inputw.o:(.data.rel.ro._ZTV7keyedit[_ZTV7keyedit]+0x78): undefined reference to `GUIedit::draw(char*)'
obj/inputw.o:(.data.rel.ro._ZTI17changeinputdevice[_ZTI17changeinputdevice]+0x10): undefined reference to `typeinfo for GUIcontents'
obj/inputw.o:(.data.rel.ro._ZTI13inputtestarea[_ZTI13inputtestarea]+0x10): undefined reference to `typeinfo for GUIrect'
obj/inputw.o:(.data.rel.ro._ZTI15inputtestbutton[_ZTI15inputtestbutton]+0x10): undefined reference to `typeinfo for GUIrect'
obj/inputw.o:(.data.rel.ro._ZTI12redefinekeys[_ZTI12redefinekeys]+0x10): undefined reference to `typeinfo for GUIcontents'
obj/inputw.o:(.data.rel.ro._ZTI7keyedit[_ZTI7keyedit]+0x10): undefined reference to `typeinfo for GUIedit'
obj/message.o: In function `MESSAGE::draw(int, int)':
/home/macdue/git/NESticlePort/src/message.cpp:15: undefined reference to `screen'
```
