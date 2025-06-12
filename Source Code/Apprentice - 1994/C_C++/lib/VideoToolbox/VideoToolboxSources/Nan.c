/*
Nan.c
This file declares a global constant that represents a double with the value NAN(004).
You access it by using the symbol "Nan", e.g.
	x=Nan;
"Nan" is defined in VideoToolbox.h as *(double *)__NAN.
The virtue of this approach, rather than just computing a NAN, e.g. 0.0/0.0,
is that it's faster. Secondly, at the moment I'm trying to work around a bug
in the THINK C Debugger (versions 6.01 and 7.0) on the PowerBook 170 that
fails when trying to compute 0.0/0.0 while Virtual Memory is enabled.
*/

#if __option(double_8)
	const short __NAN[]={ 0x7FF0, 0x0800, 0x0000, 0x0000 };
#else
	#if !__option(native_fp)
		const short __NAN[]={ 0x7FFF, 0x7FFF, 0x0004, 0x0000, 0x0000, 0x0000 };
	#elif __option(mc68881)
		const short __NAN[]={ 0x7FFF, 0x0000, 0x0004, 0x0000, 0x0000, 0x0000 };
	#else
		const short __NAN[]={ 0x7FFF, 0x0004, 0x0000, 0x0000, 0x0000 };
	#endif
#endif
