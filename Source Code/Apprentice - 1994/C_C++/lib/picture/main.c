//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		main.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	Apr. 3, 1992
*
*	main() function for pict applications.
*
*	ASSOCIATED FILES:
*		One of:
*		macscrn.c/macscrn.h, pcscrn.c/pcscrn.h, xscrn.c/xscrn.h.
*
*		One of:
*		simpict.h/simpict.c, ringpict.h/ringpict.c.,
*		anringpi.h/anringpi.c, dycubpic.h/dycubpic.c.
*
*		For simpict (simple picture):
*		class.c,class.h,screen.c,screen.h,coord.c,coord.h,
*		frame.c,frame.h,color.h,error.c,error.h,project.c,project.h,
*		backdrop.c,backdrop.h,pict.c,pict.h,several ANSI and system-
*		specific headers.
*
*		For ringpict (ring picture), above files plus:
*		trans.c,trans.h,camera.c,camera.h,segment.c,segment.h,
*		line.c,line.h,cube.c,cube.h,ring.h,ring.c.
*
*		For anringpi (animated picture), above files plus:
*		animate.h,animate.c,atring.c,atring.h,anring.c,anring.h.
*
*		For dycubpic (dynamic picture), above files plus:
*		dynamic.h,dynamic.c,dycube.h,dycube.c.
*
*	PROJECT CONTENTS (Think C):
*		above-listed source (.c) files, MacTraps, ANSI,
*		oops library.
*
*	COMPILATION (Think C):
*		68881 code generation if available.  Prefix: #include <think.h>
*/

# include	"anringpi.h"
# include	"ringpict.h"
# include	"simpict.h"
# include	"dycubpic.h"

/******************************************************************
*	main function
******************************************************************/
main()
{
	Generic_Pict	*pict;
	
	pict = new An_Ring_Pict;
	pict->run();
	delete pict;
}

	
	
