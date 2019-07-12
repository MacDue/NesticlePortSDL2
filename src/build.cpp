//build info
char buildtime[]=__TIME__;
char builddate[]=__DATE__;

#if defined(__WATCOMC__) && !defined(__GNUC__)
char buildcompiler[]="Watcom C++";
int buildcompilerversionhigh=__WATCOMC__/100;
int buildcompilerversionlow=__WATCOMC__%100;
#endif
#ifdef __BORLANDC__
char buildcompiler[]="Borland C++";
int buildcompilerversionhigh=__BORLANDC__/256;
int buildcompilerversionlow=__BORLANDC__%256;
#endif

#ifdef _MSC_VER
char buildcompiler[]="MS VC++";
int buildcompilerversionhigh=4;
int buildcompilerversionlow=10;
#endif

#ifdef __GNUG__
char buildcompiler[]="GCC";
int buildcompilerversionhigh=__GNUC__;
int buildcompilerversionlow=__GNUC_MINOR__;
#endif
