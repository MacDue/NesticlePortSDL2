
#define TIMERSPEED 60

//cycles per second
#define CYCLES     1800000

//number of cycles to execute per frame
#define CYCLESPERTICK (CYCLES/TIMERSPEED)

//percentage of vblank for a frame
#define VBLANK 13

//cycles before vblank
#define FRAMECYCLES (CYCLESPERTICK*(100-VBLANK)/100)

//cycles during vblank
#define VBLANKCYCLES (CYCLESPERTICK*VBLANK/100)
