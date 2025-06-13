/*
*    macros.c
*	 by Gaige B. Paulsen
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1992,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*	7/92	Moved here from event.c and maclook.c by JMB
*/

#ifdef MPW
#pragma segment Macros
#endif

/* Macro Defines */
#define MACRO_IP		0xff	/* Send IP number here */
#define MACRO_LINES		0xfe	/* Send # of lines here */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "TelnetHeader.h"
#include "network.proto.h"				/* For netwrite proto */
#include "wind.h"				/* For WindRec definition */
#include "DlogUtils.proto.h"
#include "translate.proto.h"
#include "parse.proto.h"
#include "ftpbin.proto.h"
#include "event.proto.h"

#include "vsdata.h"
#include "vsinterf.proto.h"

#include "macros.proto.h"

extern WindRec *screens;
extern short scrn;
extern Cursor *theCursors[];

unsigned char *macro[10];	/* the wonderful macro package .... */

void	MACROSunload(void) {}

void setmacro( short n, unsigned char *s)			/* Set macro number <n> to the value of s */
{
	unsigned char *p;
	short num=0, pos=0, escape=0;

	if (n<0  || n>9)
		return;

	p=macro[n];

	while ( *s) {
		if (escape) {
			escape=0;
			switch (*s) {
				case 'i':
					if ( pos >0) {
						*p++=num;
						*p++=*s;
						pos=0;
						}
					*p++=MACRO_IP;
					break;
				case '#':
					if ( pos >0) {
						*p++=num;
						*p++=*s;
						pos=0;
						}
					*p++=MACRO_LINES;
					break;
				case 'n':
					if ( pos >0) {
						*p++=num;
						*p++=*s;
						pos=0;
						}
					*p++='\012';
					break;
				case 'r':
					if ( pos >0) {
						*p++=num;
						*p++=*s;
						pos=0;
						}
					*p++='\015';
					break;
				case 't':
					if ( pos >0) {
						*p++=num;
						*p++=*s;
						pos=0;
						}
					*p++='\t';
					break;
				case '"':
					if ( pos >0) {
						*p++=num;
						*p++=*s;
						pos=0;
						}
					*p++='\"';
					break;

						
				case '\\':
					if ( pos >0) {
						*p++=num;
						escape=1;
						pos=0;
						num=0;
						}
					else
						*p++='\\';
					break;
				default:
					if (*s <='9' && *s >='0' && pos <3) {
						num= num*8+( *s -'0');
						pos++;
						escape=1;
						}
					else {
						if (pos ==0 && num==0) {
							*p++='\\';
							*p++=*s;
							}
						else {
							*p++=num;
							pos= 0;
							s--;			/* back up the buffer. */
							}
						}
					break;
				}
			}
		else {
			if (*s=='\\') {
				num=0;
				pos=0;
				escape=1;
				}
			else
				*p++=*s;
			}
		s++;
		}

	if (pos >0) *p++=num;
	*p=0;
} /* setmacro */

short sendmacro( short n)				/* send macro number n */
{
	unsigned char temp[300], *mp, *tp;
	unsigned char myipnum[4];

	if (n<0 || n>9) return(-1);

	tp = temp;
	mp = macro[n];
	netgetip(myipnum);
	while ( *mp) {
		if ( *mp==MACRO_IP) {
			sprintf((char *) tp,"%d.%d.%d.%d", myipnum[0], myipnum[1], myipnum[2], myipnum[3]);		/* BYU LSC */
			tp+=strlen((char *) tp);	/* BYU LSC */
			mp++;
			}
		else if ( *mp==MACRO_LINES) {
			sprintf((char *) tp,"%d", VSgetlines( screens[scrn].vs));	/* BYU LSC */
			tp+=strlen((char *) tp);	/* BYU LSC */
			mp++;
			}
		else if (screens[scrn].ftpstate) {								/* BYU 2.4.16 */
			if (*mp == CR) {											/* BYU 2.4.16 */
				parse( &screens[scrn],(unsigned char *) "\015\012",2);	/* BYU 2.4.16 */
				screens[scrn].kbbuf[ screens[scrn].kblen++ ] = 0;		/* BYU 2.4.16 */
				ftppi(screens[scrn].kbbuf);								/* BYU 2.4.16 - ftp client */
				screens[scrn].kblen=0;									/* BYU 2.4.16 */
				mp++;													/* BYU 2.4.16 */
			} else {													/* BYU 2.4.16 */
				screens[scrn].kbbuf[ screens[scrn].kblen++ ] = *mp;		/* BYU 2.4.16 */
				parse( &screens[ scrn], mp++, 1);						/* BYU 2.4.16 */
			}															/* BYU 2.4.16 */
		} else *tp++=*mp++;												/* BYU 2.4.16 */
	}																	/* BYU 2.4.16 */
	*tp=0;						/* Gotta have a nul! */
	tp= temp;

	trbuf_nat_mac((unsigned char *)tp, strlen((char *)tp), screens[scrn].national);		/* LU */

	if (!screens[scrn].ftpstate) {		/* BYU 2.4.16 */

		netpush( screens[scrn].port);									/* BYU 2.4.16 */

		if (screens[scrn].lmode) 	/* need to flush buffer */			/* BYU 2.4.16 */
			{															/* BYU 2.4.16 */
			netwrite(screens[scrn].port,screens[scrn].kbbuf,screens[scrn].kblen);	/* BYU 2.4.16 */
			screens[scrn].kblen=0;										/* BYU 2.4.16 */
			}															/* BYU 2.4.16 */
	}																	/* BYU 2.4.16 */

	if (!screens[scrn].ftpstate) {										/* BYU 2.4.16 */
		netwrite(screens[scrn].port, tp, strlen((char *) tp) );			/* BYU LSC */
	
		if (screens[scrn].echo)
			parse( &screens[scrn],tp, strlen((char *) tp) );			/* BYU LSC */
	}																	/* BYU 2.4.16 */

	return(0);
}

short getmacro( short n, unsigned char *dest)
{
	unsigned char *s;

	if (n<0 || n>9)
		return(-1);

	s = macro[n];
	while (*s) {
		switch( *s) {
		case MACRO_IP :
			*dest++='\\';
			*dest++='i';
			break;
		case MACRO_LINES :
			*dest++='\\';
			*dest++='#';
			break;
		case '\\':
			*dest++='\\';
			*dest++='\\';
			break;
		case '\015':
			*dest++='\\';
			*dest++='r';
			break;
		case '\012':
			*dest++='\\';
			*dest++='n';
			break;
			
#ifdef OLD_CRAP
		case '\015':
			s++;
			if (*s == '\012' || *s=='\000') {
				*dest++='\\';
				*dest++='n';
				}
			else {
				*dest++='\\';
				*dest++='0';
				*dest++='1';
				*dest++='5';
				s--;
				}
			break;
#endif
		case '\t':
			*dest++='\\';
			*dest++='t';
			break;
		default: 
			if ( isprint(*s)) 
				*dest++=*s;
			else {
				*dest++='\\';
				*dest++= (*s / 64) +'0';
				*dest++= ((*s % 64) / 8)+'0';
				*dest++= (*s % 8) +'0';
				}
			break;
			}
		s++;
		}
	*dest=0;
	return( 0);
}

void Macros( void)
{
	DialogPtr dtemp;
	short dItem;
	short i;
	Rect dBox;
	Str255 temp;
	Handle MacString[10];

	SetCursor(theCursors[normcurs]);

	dtemp=GetNewMyDialog( MacroDLOG, NULL, kInFront, (void *)ThirdCenterDialog);

	for (i=0; i<10; i++) {
		getmacro( i, (unsigned char *) &temp);		/* BYU LSC */
		c2pstr((char *)temp);								/* BYU LSC */
		GetDItem( dtemp, i+13, &dItem, &MacString[i], &dBox);
		SetIText( MacString[i], temp );
		}

	dItem=0;								/* initially no hits */
	while((dItem>2) || (dItem==0)) {		/* While we are in the loop */
		ModalDialog(DLOGwOK_CancelUPP,&dItem);
		if (dItem >2 && dItem <13) {
			i=dItem-3;
			getmacro( i, (unsigned char *) &temp);			/* BYU LSC */
			c2pstr((char *)temp);
			GetDItem( dtemp, i+13, &dItem, &MacString[i], &dBox);
			SetIText( MacString[i], temp );				/* BYU LSC - Revert the mother */
			SelIText( dtemp, i+13, 0, 32767);				/* And select it... */
			}
		}
		
	updateCursor(1);
	
	if (dItem==DLOGCancel) {
			DisposDialog( dtemp);
			return;
			}

	for (i=0; i<10; i++) {
		GetIText( MacString[i], temp);
		p2cstr(temp);
		setmacro(i, (unsigned char *) &temp);
		}

	DisposDialog( dtemp);
}
