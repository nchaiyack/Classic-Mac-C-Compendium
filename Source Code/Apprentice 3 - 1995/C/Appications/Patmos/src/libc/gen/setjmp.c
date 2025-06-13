#include <setjmp.h>

void longjmp(jmp_buf saved, int result)
	{
	asm        (
            "        movel	a6@(8),a0\n"
            "        movel	a6@(12),d0\n"
            "        tstl	d0\n"
            "        bne	_1\n"
            "        moveq	#1,d0\n"
            "_1:     moveml	a0@,d1-d7/a1-a7\n"
            "        jmp	a1@\n"
            );
  	}
	
int setjmp (jmp_buf save)
	{
	asm        ("unlk       a6\n"
            "        movel	a7@+,a1\n"
            "        movel	a7@,a0\n"
	    	"        moveq      #0,d0\n"
            "        moveml    d1-d7/a1-a7,a0@\n"
            "        jmp        a1@\n"
            );
	}

void _longjmp(jmp_buf saved, int result)
	{
	asm        (
            "        movel	a6@(8),a0\n"
            "        movel	a6@(12),d0\n"
            "        tstl	d0\n"
            "        bne	_1\n"
            "        moveq	#1,d0\n"
            "_2:     moveml	a0@,d1-d7/a1-a7\n"
            "        jmp	a1@\n"
            );
  	}
	
int _setjmp (jmp_buf save)
	{
	asm        ("unlk       a6\n"
            "        movel	a7@+,a1\n"
            "        movel	a7@,a0\n"
	    	"        moveq      #0,d0\n"
            "        moveml    d1-d7/a1-a7,a0@\n"
            "        jmp        a1@\n"
            );
	}

