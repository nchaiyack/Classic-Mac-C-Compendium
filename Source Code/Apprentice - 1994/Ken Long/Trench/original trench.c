/* Written 12:00 pm  Jul 16, 1985 by sdh@joevax.UUCP in uiucdcs:net.sources.mac */
/* ---------- "source to trench simulator" ---------- */
/*This is the source to the trench simulator. It was written in MegaMax C*/
/* Note the tabs should be set to 4 spaces. */

/* Steve Hawley */
/* {joevax,mouton,alice}!sdh */

/* Program to simulate the trench from Star Wars */
/* Written 7/12/85 by Steve Hawley in MegaMax C  */

#include <qd.h>			/*QuickDraw header file */
#include <qdvars.h>	/*Quickdraw variables */
#include <event.h>  /*Event manager header */
#include <win.h>		/*Window manager header */

windowrecord wrecord; /*Record for window */
windowptr mywindow;		/*pointer to record */
rect screenrect;			/*Rectangle representing window size*/

int wlines = 0;				/* phase of depth lines */

drawlines(offx, offy, start) /* draws lines to give illusion of depth */
	int offx, offt, start;
{
	int x1 = -200, y1 = -100;
	int z;

/* start is the phase (from 0 to 3), The lines are projected by */
/* the formulae x' = x/z; y' = y/z. offx and offy are offsets for*/
/* viewpoint changes */
	
	for (index = 50 - start; index > 0; index -= 4)
	{
		moveto( (x1 - offx)/z, (y1 - offy)/z);
		lineto( (x1 - offx)/z, (-y1 - offy)/z);
		moveto( (-x1 - offx)/z, (y1 - offy)/z);
		lineto( (-x1 - offx)/z, (-y1 - offy)/z);
	}
}

setup(offx, offy) /* draws the frame of the trench */
/* offx and offy again represent the viewpoint offsets, and it is */
/* projected using the same formulae as before */
	int offx, offy;
{

	int x1 = -200, x2 = -100, y1 = -100;
	
	
	
	moveto(x1 - offx,y1 -offy);
	lineto((x1 - offx)/50, (y1-offy)/50);
	lineto((x1 - offx)/50, (-y1 - offy)/50);
	lineto(x1 - offx, -y1 - offy);
	moveto(x2 - offx, -y1 - offy);
	lineto((x2 - offx)/50, (-y1 - offy)/50);
	moveto(-x1 - offx,y1 -offy);
	lineto((-x1 - offx)/50, (y1-offy)/50);
	lineto((-x1 - offx)/50, (-y1 - offy)/50);
	lineto(-x1 - offx, -y1 - offy);
	moveto(-x2 - offx, -y1 - offy);
	lineto((-x2 - offx)/50, (-y1 - offy)/50);
}

main()
{
/* the objects are animated by using exclusive-or drawing. this way, the */
/* same routine to draw can be used to erase, and the new image can be */
/* drawn before the old image is erased to help eliminate flicker. Thus*/
/* the the program needs two copies of the offset parameters, one for the*/
/* new and one for the old. */
	int offxo=0,offxn=0,offyo=0,offyn=0;
	point mouse;
	
	initgraf(&theport);	/* Set up quickdraw */
	flushevents(everyevent, 0); /*kill previous events*/
	initwindows();		/*initialize window manager*/
	setrect(&screenrect, 4, 40, 508, 338); /* Set screen size*/
	mywindow = newwindow(&wrecord, &screenrect, "Trench",1,0,(long)-1,(long)0);
/*Set window parameters */
	setport(mywindow); /*make the window the current grafport */
	showwindow(mywindow); /*display window */
	setorigin(-252, -149);/* set the origin so the center of the screen in (0,0)*/
	penmode(patxor); /* set exclusive-or drawing*/
	setup(offxn,offyn); /*draw initial setup*/
	drawlines(offxn, offyn, wlines); /*draw initiail depth lines */
	while(!button()) /*repeat until button is down*/
	{
		offxo = offxn;   /* Put new offsets in old variables */
		offyo = offyn;
		getmouse(&mouse); /*get the mouses coordinates */
		if ( mouse.vh[1]/2 < offxn)  /* if the horizontal has changed */
			offxn =  mouse.vh[1];			 /* store it in the new offset */
		else if ( mouse.vh[1]/2 > offxn)
			offxn = mouse.vh[1]/2;
		if (mouse.vh[0]/2 < offyn)   /* if the vertical has changed */
			offyn = mouse.vh[0]/2;		 /* stroe it in the new offset */
		else if (mouse.vh[0]/2 > offyn)
			offyn = mouse.vh[0]/2;
		if ( (offxo != offxn) || (offyo != offyn)) /* if the old offset*/
		{																				/*differs from the new, update */
			setup(offxn, offyn); /*draw new setup */
			setup(offxo, offyo); /*erase old */
		}
		drawlines(offxo, offyo, wlines); /* erase the vertical lines */
		wlines++; /* increment wlines */
		if (wlines > 3) wlines = 0; /* reset wlines if too big */
		drawlines(offxn, offyn, wlines); /* draw new set of lines */
	}
}
	
/* End of text from uiucdcs:net.sources.mac */
