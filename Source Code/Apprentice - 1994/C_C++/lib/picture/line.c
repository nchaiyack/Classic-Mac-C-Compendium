//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		line.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 6, 1990
*
*	defines methods for 3D line segment
*/

# include	"line.h"

/******************************************************************
*	initialize
******************************************************************/
Lline::Lline(void)
{
	c1 = new Coord3;
	c2 = new Coord3;
	set_coord(10.,0.,0.,10.,0.,1.);
	set_color(WHITE);
}

/******************************************************************
*	set coordinates
******************************************************************/
void	Lline::set_coord(double x1,double y1,double z1,
				double x2,double y2,double z2)
{
	c1->x = x1;
	c1->y = y1;
	c1->z = z1;
	c2->x = x2;
	c2->y = y2;
	c2->z = z2;
}
	
/******************************************************************
*	set color
******************************************************************/
void	Lline::set_color(color line_color_val)
{
	line_color = line_color_val;
}
	
/******************************************************************
*	draw 3D line
******************************************************************/
void	Lline::draw(Camera* camera,Projector* projector,
					Transformation* trans)
{
	boolean		success = TRUE;
	Coord2		*image1,
				*image2;
	Coord3		*new_coord;
	
	image1 = new Coord2;
	image2 = new Coord2;
	new_coord = new Coord3;
	
	new_coord->apply(c1,trans);
	if (!camera->take_photo(image1,new_coord))
		success = FALSE;
	new_coord->apply(c2,trans);
	if (!camera->take_photo(image2,new_coord))
		success = FALSE;
	if (success)
		projector->show_line(image1,image2,line_color);
	
	delete image1;
	delete image2;
	delete new_coord;
}

/******************************************************************
*	move line coordinates
******************************************************************/
void	Lline::move(Transformation* trans)
{
	Coord3	*temp;
	
	temp = new Coord3;
	temp->equate(c1);
	c1->apply(temp,trans);
	temp->equate(c2);
	c2->apply(temp,trans);
	delete temp;
}

/******************************************************************
*	destroy
******************************************************************/
Lline::~Lline(void)
{
	delete c1;
	delete c2;
}


