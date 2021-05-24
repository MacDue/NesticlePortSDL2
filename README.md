# NESticlePort
An experiment porting (reverse engineered) NESticle code released by [IamRifki](https://github.com/IamRifki/NESticle) to [SDL2](https://www.libsdl.org/).

## Status

**NOTE: I am not actively working on this project! If you wish to continue it please do!**

I have the Emulator somewhat working!

Screenshot of Nesticle running on Ubuntu 20.04:

![The emulator](./github_images/screenshot.png)

This is mostly using the original code with some fixes, and some compiled code reverse engineered.

### What works

- The GUI and mouse input (mostly)
- Loading ROMs
- Running games (somewhat -- there's a lot of emulation issues)

### What's left todo
  - Keyboard input/controls
  - Getting the emulator to a playable state
  - Probably more

### How to compile?

  Install SDL2 then run
  ```
  CC=g++ make nesticle  # note compiling the code as 64bit works!
  ./nesticle            # run the emulator (needs to be in the same folder as gui.vol and anes.pal)
  ```
  (this is on a Linux system I've yet to attempt building on windows but it should work with mingw64)

  **Note:** You need to use at least `g++ 7` as I used C++17's ``std::filesystem`` to reimplement the ROM loader.
