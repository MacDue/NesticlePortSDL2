extern void InitializeTimers();
extern void TerminateTimers();
extern void __cdecl SetTimerSpeed(unsigned int x);
extern void __cdecl SetMusicSpeed(unsigned int x);
extern void (*timerhandler)(void);  //pointer to our handler
extern void (*musichandler)(void);  //pointer to the music handler
extern unsigned speed;
extern unsigned mspeed;
extern void __cdecl SetTimerFunc(void (*func)(void));
extern void __cdecl SetMusicFunc(void (*func)(void));

void _enter(int area);
void _leave();
void _proftimer();
extern int area[16];
extern int profticks;

