/* Copyright 1994 Ralph Gonzalez */

/*
*	FILE:		cube3d.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	March 16, 1994
*
*	Sample application using mgraph/xgraph and trans routines.
*
*	PROJECT CONTENTS:
*		cube3d.c/.cc, mgraph.c/xgraph.cc, trans.c/.cc
*/

# ifdef THINK_C
# include	"mgraph.h"
# else
# include	"xgraph.h"
# endif
# include	"trans.h"
# include	<stdio.h>

# ifndef PI
# define	PI	3.1415926
# endif

void		initialize_cube(Coord3*);
void		transform_cube(Coord3*,double[4][4]);
void		draw_cube(Coord3*,double[4][4]);

main()
{
	Coord3	cube[8];
	double	t[4][4],
			ct[4][4],
			wt[4][4];
	Camera	c;
	double	mouse_x,
			mouse_y;
	
	init_graphics();	/* Don't forget to do this FIRST! */
	printf("Mouse position determines velocity, click when done.\n");
	background_color(BLUE);
	erase_graphics();
	
	initialize_cube(cube);
	set_translation(t,-.5,-.5,-.5);	/* center cube on origin */
	transform_cube(cube,t);
	
	/* camera is positioned on the x axis, facing the origin: */
	set_camera(&c,5.,0.,0.,3.*PI/2.,0.,0.);
	set_world_to_camera(wt,c);
	draw_cube(cube,wt);
	
	while (!mouse_button_is_down())
	{
		get_mouse_location(&mouse_x,&mouse_y);
		set_rotation_y(ct,mouse_x*PI/10.);
		set_rotation_z(t,mouse_y*PI/10.);
		combine_trans(ct,t);
		transform_cube(cube,ct); /* apply compound transformation */
		erase_graphics();
		draw_cube(cube,wt);
	}
}

/******************************************************************
*	create cube
******************************************************************/
void	initialize_cube(Coord3 p[])
{
	set_coord(&p[0],0.,0.,0.);
	set_coord(&p[1],1.,0.,0.);
	set_coord(&p[2],1.,1.,0.);
	set_coord(&p[3],0.,1.,0.);
	set_coord(&p[4],0.,0.,1.);
	set_coord(&p[5],1.,0.,1.);
	set_coord(&p[6],1.,1.,1.);
	set_coord(&p[7],0.,1.,1.);
}

/******************************************************************
*	transform cube
******************************************************************/
void	transform_cube(Coord3 p[],double t[4][4])
{
	int		i;
	
	for (i=0 ; i<8 ; i++)
		apply_trans(&p[i],t);
}

/******************************************************************
*	project and draw 12 lines of cube. Doesn't draw cube if any
*	point is behind the camera.
******************************************************************/
void	draw_cube(Coord3 p[],double t[4][4])
{
	int		i;
	Coord2	pict[8];
	boolean	in_front = TRUE;
	
	for (i=0 ; i<8 && in_front ; i++)
		in_front = project(&pict[i],p[i],t,4.);
		
	if (in_front)
	{
		draw_line(pict[0].x,pict[0].y,pict[1].x,pict[1].y);
		draw_line(pict[1].x,pict[1].y,pict[2].x,pict[2].y);
		draw_line(pict[2].x,pict[2].y,pict[3].x,pict[3].y);
		draw_line(pict[3].x,pict[3].y,pict[0].x,pict[0].y);
		draw_line(pict[4].x,pict[4].y,pict[5].x,pict[5].y);
		draw_line(pict[5].x,pict[5].y,pict[6].x,pict[6].y);
		draw_line(pict[6].x,pict[6].y,pict[7].x,pict[7].y);
		draw_line(pict[7].x,pict[7].y,pict[4].x,pict[4].y);
		draw_line(pict[0].x,pict[0].y,pict[4].x,pict[4].y);
		draw_line(pict[1].x,pict[1].y,pict[5].x,pict[5].y);
		draw_line(pict[2].x,pict[2].y,pict[6].x,pict[6].y);
		draw_line(pict[3].x,pict[3].y,pict[7].x,pict[7].y);
	}
}
