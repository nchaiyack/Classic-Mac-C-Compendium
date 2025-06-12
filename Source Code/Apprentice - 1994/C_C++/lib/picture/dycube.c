//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		dycube.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	Nov. 25, 1991
*
*	defines methods of Cube dynamic segment
*/

# include	"dycube.h"
# include	"line.h"
# include	"trans.h"

/******************************************************************
*	initialize cube
******************************************************************/
Dynamic_Cube::Dynamic_Cube(void)
{
	int		i;
	
	num_segments = 12;
	
	for (i=0 ; i<12 ; i++)
	{
		segment[i] = new Lline;
		segment[i]->set_color(WHITE);
	}
	
	((Lline*) segment[0])->set_coord(0.,0.,0.,0.,0.,1.);
	((Lline*) segment[1])->set_coord(0.,0.,1.,0.,1.,1.);
	((Lline*) segment[2])->set_coord(0.,1.,1.,0.,1.,0.);
	((Lline*) segment[3])->set_coord(0.,1.,0.,0.,0.,0.);
	((Lline*) segment[4])->set_coord(1.,0.,0.,1.,0.,1.);
	((Lline*) segment[5])->set_coord(1.,0.,1.,1.,1.,1.);
	((Lline*) segment[6])->set_coord(1.,1.,1.,1.,1.,0.);
	((Lline*) segment[7])->set_coord(1.,1.,0.,1.,0.,0.);
	((Lline*) segment[8])->set_coord(0.,0.,0.,1.,0.,0.);
	((Lline*) segment[9])->set_coord(0.,0.,1.,1.,0.,1.);
	((Lline*) segment[10])->set_coord(0.,1.,1.,1.,1.,1.);
	((Lline*) segment[11])->set_coord(0.,1.,0.,1.,1.,0.);

	set_center(.5,.5,.5);
}

/******************************************************************
*	initialize fast dynamic cube
******************************************************************/
Fast_Dynamic_Cube::Fast_Dynamic_Cube(void)
{
	int		i;
	
	for (i=0 ; i<8 ; i++)
		c[i] = new Coord3;
	
	c[0]->set(0.,0.,0.);
	c[1]->set(0.,0.,1.);
	c[2]->set(0.,1.,1.);
	c[3]->set(0.,1.,0.);
	c[4]->set(1.,0.,0.);
	c[5]->set(1.,0.,1.);
	c[6]->set(1.,1.,1.);
	c[7]->set(1.,1.,0.);
	
	set_color(WHITE);
	
	set_center(.5,.5,.5);
}

/******************************************************************
*	color fast dynamic cube
******************************************************************/
void	Fast_Dynamic_Cube::set_color(color cube_color_val)
{
	cube_color = cube_color_val;
}

/******************************************************************
*	draw fast dynamic cube
******************************************************************/
void	Fast_Dynamic_Cube::draw(Camera* camera,Projector* projector,
					Transformation* trans)
{
	int			i;
	boolean		success = TRUE;
	Coord2		*image[8];
	Coord3		*new_coord;
		
	new_coord = new Coord3;

	for (i=0 ; i<8 ; i++)
	{
		image[i] = new Coord2;
		new_coord->apply(c[i],trans);
		if (!camera->take_photo(image[i],new_coord))
			success = FALSE;
	}
	
	if (success)
	{	
		projector->show_line(image[0],image[1],cube_color);
		projector->show_line(image[1],image[2],cube_color);
		projector->show_line(image[2],image[3],cube_color);
		projector->show_line(image[3],image[0],cube_color);
		projector->show_line(image[0],image[4],cube_color);
		projector->show_line(image[4],image[5],cube_color);
		projector->show_line(image[5],image[6],cube_color);
		projector->show_line(image[6],image[7],cube_color);
		projector->show_line(image[7],image[4],cube_color);
		projector->show_line(image[1],image[5],cube_color);
		projector->show_line(image[2],image[6],cube_color);
		projector->show_line(image[3],image[7],cube_color);
	}
	
	for (i=0 ; i<8 ; i++)
		delete image[i];

	delete new_coord;
}

/******************************************************************
*	move dynamic cube coordinates.  Since we never use the
*	Fast_Dynamic_Cube's "transformation" instance variable,
*	there's no need to call inherited move() method.  But
*	we must remember to move the inherited center as well.
******************************************************************/
void	Fast_Dynamic_Cube::move(Transformation* trans)
{
	Coord3	*temp;
	int		i;
	
	temp = new Coord3;
	
	for (i=0 ; i<8 ; i++)
	{
		temp->equate(c[i]);
		c[i]->apply(temp,trans);
	}

	temp->equate(center);
	center->apply(temp,trans);
	
	delete temp;
}

/******************************************************************
*	destroy dynamic cube
******************************************************************/
Fast_Dynamic_Cube::~Fast_Dynamic_Cube(void)
{
	int		i;
	
	for (i=0 ; i<8 ; i++)
		delete c[i];
}


