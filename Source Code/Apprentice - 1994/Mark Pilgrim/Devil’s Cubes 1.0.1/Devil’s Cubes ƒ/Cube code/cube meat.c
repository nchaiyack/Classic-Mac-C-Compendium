/**********************************************************************\

File:		cube meat.c

Purpose:	This module handles internal variable manipulation for
			keeping track of cube rotations.


Devil’s Cubes -- a simple cubes puzzle
Copyright (C) 1993 Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "cube meat.h"
#include "cube.h"

void Qrotate(int i)
{
	int			temp;
	
	temp=Cube[i][4];
	Cube[i][4]=Cube[i][1];
	Cube[i][1]=Cube[i][5];
	Cube[i][5]=Cube[i][3];
	Cube[i][3]=temp;
}

void Xrotate(int i)
{
	int			temp;
	
	temp=Cube[i][4];
	Cube[i][4]=Cube[i][3];
	Cube[i][3]=Cube[i][5];
	Cube[i][5]=Cube[i][1];
	Cube[i][1]=temp;
}

void Srotate(int i)
{
	int			temp;
	
	temp=Cube[i][0];
	Cube[i][0]=Cube[i][5];
	Cube[i][5]=Cube[i][2];
	Cube[i][2]=Cube[i][4];
	Cube[i][4]=temp;
}

void Arotate(int i)
{
	int			temp;
	
	temp=Cube[i][0];
	Cube[i][0]=Cube[i][4];
	Cube[i][4]=Cube[i][2];
	Cube[i][2]=Cube[i][5];
	Cube[i][5]=temp;
}

void Wrotate(int i)
{
	int			temp;
	
	temp=Cube[i][0];
	Cube[i][0]=Cube[i][1];
	Cube[i][1]=Cube[i][2];
	Cube[i][2]=Cube[i][3];
	Cube[i][3]=temp;
}

void Zrotate(int i)
{
	int			temp;
	
	temp=Cube[i][0];
	Cube[i][0]=Cube[i][3];
	Cube[i][3]=Cube[i][2];
	Cube[i][2]=Cube[i][1];
	Cube[i][1]=temp;
}
