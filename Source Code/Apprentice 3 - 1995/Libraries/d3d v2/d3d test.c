// d3d test.c
// Darrell Anderson

#include "d3d_v2.h"
#include "d3d_v2_debug.h"
#include "tankObject.h"
#include <stdio.h>
#include <events.h>

#ifndef SCALE
#define SCALE 100.0
#endif

void main(void) {
	d3dPoint tetrahedron[4];
	d3dPoint tetrahedron2[4];
	d3dPoint origP, finalP, origO, finalO;
	d3dPoint P;
	short i,j;
	
	d3dMatrix M,M2,M3;
	d3dPoint P2,P3;
	
	WindowPtr win;
	Rect box;
	GrafPtr oldPort;
	long finalTick;
	Point mouse;
	
	d3dPoint tankP, tankO;
	d3dMatrix tankPM, tankOM;
	EventRecord event;
	
	short x,y;
	
	double xang;
	
	TankObj us, them;

	d3dPoint grid[10][10];
	d3dPoint grid2[10][10];
	
		for( i=0 ; i<10 ; i++ ) {
			for( j=0 ; j<10 ; j++ ) {
				grid[i][j].x = i;
				grid[i][j].y = -1;
				grid[i][j].z = j;
			}
		}
	
	// toolbox!
	InitGraf( &qd.thePort ); InitFonts(); InitWindows();
	InitMenus(); TEInit(); InitDialogs( nil ); InitCursor();
	
	printf("start.\n");
	
	tetrahedron[0].x =  1;
	tetrahedron[0].y =  1;
	tetrahedron[0].z = -1;
	
	tetrahedron[1].x =  1;
	tetrahedron[1].y = -1;
	tetrahedron[1].z =  1;
	
	tetrahedron[2].x = -1;
	tetrahedron[2].y =  1;
	tetrahedron[2].z =  1;
	
	tetrahedron[3].x = -1;
	tetrahedron[3].y = -1;
	tetrahedron[3].z = -1;
		
	origP.x = 0;
	origP.y = 0;
	origP.z = 0;

	origO.x = 0;
	origO.y = 0;
	origO.z = 0;

	finalP.x = 0;
	finalP.y = 0;
	finalP.z = 0;

	finalO.x = 0;
	finalO.y = 0;
	finalO.z = 0;
	
	//---
	
	d3dMakeIdentity(&M);
	printf("M:\n");
	PrintMatrix(&M);
	
	d3dGetRotation(&M2, PI/2.0, 0, 0);
	printf("M2:\n");
	PrintMatrix(&M2);
	
//	d3dMultMatrixMatrix(&M3,&M2,&M);
//	printf("M2 x M:\n");
//	PrintMatrix(&M2);

	P.x = 5;
	P.y = 6;
	P.z = 7;
	
	d3dMultMatrixPoint( &P2, &M2, &P );
	printf( "\nP2: " );
	PrintPoint(&P2);
	
	//---
	GetPort(&oldPort);
	SetRect(&box, 20,40,220,240);
	win = NewWindow(nil,&box,"\p",1,0,(WindowPtr)-1L,0,0L);
	SetPort(win);
	
	xang = 0;
	
	tankP.x = 0;
	tankP.y = 0;
	tankP.z = -4;

	tankO.x = 0.01;
	tankO.y = 0;
	tankO.z = 0.01;

	d3dGetRotation(&tankOM, tankO.x, tankO.y, tankO.z);
	d3dGetTranslation(&tankPM, tankP.x, tankP.y, tankP.z);

	CreateMasterTank();
		
	while( !Button() ){

/*		
		//d3dGetRotation(&M,xang,xang,0);
		
		d3dGetTranslation(&M2,0,0,5);
		
		GetMouse(&mouse);
		d3dGetRotation(&M,-((double)mouse.v)/50.0,((double)mouse.h)/50.0,0);
		
		d3dMultMatrixMatrix(&M3,&M2,&M);
		for( i=0 ; i<4 ; i++ ) {
			d3dMultMatrixPoint(&tetrahedron2[i], &M3, &tetrahedron[i]);
		}
		
		// draw it!
		Delay(1L,&finalTick);
		EraseRect(&box);
		for( i=0 ; i<4 ; i++ ) {
			//PrintPoint(&tetrahedron2[i]);
			for( j=0 ; j<4 ; j++ ) {
				if( (i!=j) ) {
					MoveTo(100,100);
					Line(0,0);
					
					MoveTo( 
						tetrahedron2[i].x / tetrahedron2[i].z * SCALE + 100, 
						tetrahedron2[i].y / tetrahedron2[i].z * SCALE + 100 );
					LineTo( 
						tetrahedron2[j].x / tetrahedron2[j].z * SCALE + 100, 
						tetrahedron2[j].y / tetrahedron2[j].z * SCALE + 100 );
				}
			}
		}
		
		xang += 0.01;
*/
/**/
		WaitNextEvent(everyEvent,&event,10, nil);

		if( event.what == keyDown || event.what == autoKey ) {
			//printf("got '%c'\n",event.message & charCodeMask);
			switch( event.message & charCodeMask ){
				case 'j':
					//d3dGetRotation(&M,0,-0.1,0);
					//d3dMultMatrixMatrix(&M2,&tankOM,&M);
					//tankOM=M2;
					tankO.y -= 0.1;
					break;
				case 'l':
					//d3dGetRotation(&M,0, 0.1,0);
					//d3dMultMatrixMatrix(&M2,&tankOM,&M);
					//tankOM=M2;
					tankO.y += 0.1;
					break;
				case 'i':
					P.x = 0;
					P.y = 0;
					P.z = 0.1;
					
					d3dGetRotation(&M,tankO.x,tankO.y,tankO.z);
					d3dMultMatrixPoint(&P2,&M,&P);

					tankP.x += P2.x;
					tankP.y += P2.y;
					tankP.z += P2.z;
					
					break;
				case 'k':
					P.x = 0;
					P.y = 0;
					P.z = -0.1;
					
					d3dGetRotation(&M,tankO.x,tankO.y,tankO.z);
					d3dMultMatrixPoint(&P2,&M,&P);

					tankP.x += P2.x;
					tankP.y += P2.y;
					tankP.z += P2.z;
					
					break;
			}
		}
/**/		
/*
		GetMouse(&mouse);
		tankP.z = -4.0 - ((double)mouse.v) / 20.0;
		tankO.y = ((double)mouse.h) / 20.0;
*/		
		// compute it!
//		for( i=0 ; i<4 ; i++ ) {
//			d3dObserve( &tetrahedron2[i], &tetrahedron[i], &tankP, &tankO );
//		}
		d3dObserveMany(tetrahedron2, tetrahedron, &tankP, &tankO, 4);		
/*	
	 	// info
		printf("\ntankP=");
		PrintPoint(&tankP);
		printf("tankO=");
		PrintPoint(&tankO);		
*/		
		// draw it!
		EraseRect(&win->portRect);
/*
		for( i=0 ; i<4 ; i++ ) {
			for( j=0 ; j<4 ; j++ ) {
				if( (i!=j) ) {
					MoveTo(100,100);
					Line(0,0);
				
					d3dMoveTo( &(tetrahedron2[i]) );
					d3dLineTo( &(tetrahedron2[j]) );

				//	MoveTo( 
				//		tetrahedron2[i].x / tetrahedron2[i].z * SCALE + 100, 
				//		tetrahedron2[i].y / tetrahedron2[i].z * SCALE + 100 );
				//	LineTo( 
				//		tetrahedron2[j].x / tetrahedron2[j].z * SCALE + 100, 
				//		tetrahedron2[j].y / tetrahedron2[j].z * SCALE + 100 );
				}
			}
		}
*/
				
		us.pos = tankP;
		us.ort = tankO;
		us.color = greenColor;
		them.pos.x = 5; them.pos.y = 0; them.pos.z = 5;
		them.ort.x = 0; them.ort.y = 0; them.ort.z = 0;
		them.color = blueColor;
		
		DrawTank(&them,&us);
		
		ForeColor(redColor);
/*		for( i=-10 ; i<10 ; i++ ) {
			for( j=-10 ; j<10 ; j++ ) {
				P.x = i;
				P.y = -1;
				P.z = j;
				d3dObserve( &P2, &P, &tankP, &tankO );
				if( P2.z > EPSILON ) {
					d3dMoveTo(&P2);
				//	MoveTo( 
				//		P2.x / P2.z * SCALE + 100,
				//		P2.y / P2.z * SCALE + 100 );
					Line(0,0);
				}
			}
		}
*/
		d3dObserveMany((d3dPoint *)grid2, (d3dPoint *)grid,&tankP,&tankO,100);
		for( i=0 ; i<10 ; i++ ) {
			for( j=0 ; j<10 ; j++ ) {
				if(grid2[i][j].z>0) {
					d3dMoveTo(&(grid2[i][j]));
					Line(0,0);
				}
			}
		}
		//ForeColor(blackColor);
		
	}
	SetPort(oldPort);
	DisposeWindow(win);
	
	printf( "end.\n" );
}

