/*/
     Project Arashi: VAColor.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:41
     Created: Tuesday, February 26, 1991, 20:45

     Copyright � 1991-1992, Juri Munkki
/*/

#include "VA.h"

static	int			*ColorMapping[2];
static	ColorSpec	VAColors[NUMCOLORS];
		ColorSpec	*VASpec[2];				/*	VA color tables.	*/
		RGBColor	VAIndexor={'IN','DX',0};

void	VACreateColorMapping()
{
	register	int		i,j;

	ColorMapping[0]=(int *)NewPtr(sizeof(int)*512L);
	ColorMapping[1]=ColorMapping[0]+256;

	for(j=0;j<2;j++)
	{	for(i=  0   ;i<128    ;i++)		ColorMapping[j][i]= 7;
		for(i=128   ;i<128+64 ;i++)		ColorMapping[j][i]= 8;
		for(i=128+64;i<128+128;i++)		ColorMapping[j][i]= 9;
	}

	for(i=0;i<256;i++)
	{	if((i & 7) != 7)				ColorMapping[0][i]= i & 7;
		if(((i >> 3) & 7) != 7)			ColorMapping[1][i]= (i>>3) & 7;
	}
}

unsigned int	MaxValue(thecolor)
register	RGBColor	*thecolor;
{
	if(thecolor->red > thecolor->blue)
	{	if(thecolor->red > thecolor->green)
			return	thecolor->red;
		else
			return	thecolor->green;
	}
	else
	{	if(thecolor->blue > thecolor->green)
			return	thecolor->blue;
		else
			return	thecolor->green;
	}
}

void	VASetColors(theColors)
Handle	theColors;
{
	register	int			*clor;
	register	ColorSpec	*ClotP;
	register	int			i,j;

	BlockMove(*theColors,VAColors,sizeof(ColorSpec)*(long)NUMCOLORS);
	
	if(VA.monochrome)
	{	for(i=0;i<NUMCOLORS;i++)
		{	VAColors[i].rgb.red		=
			VAColors[i].rgb.green	=
			VAColors[i].rgb.blue 	=	MaxValue(&VAColors[i].rgb);
		}
	}

	for(j=0;j<2;j++)
	{	clor=ColorMapping[j];
		ClotP=VASpec[j];
		for(i=0;i<256;i++)
		{	*ClotP++ = VAColors[*clor++];
		}
	}
}

Handle	VAGetColors()
{
	Handle	thecolors;
	
	thecolors=NewHandle(sizeof(ColorSpec)*(long)NUMCOLORS);
	BlockMove(VAColors,*thecolors,sizeof(ColorSpec)*(long)NUMCOLORS);
	
	return thecolors;
}

pascal
int		MyQDColorSearchProc(rgb,position)
RGBColor	*rgb;
long		*position;
{
	if(rgb->red=='IN' && rgb->green=='DX')
	{	*position = rgb->blue;
		return -1;
	}
	else
		return	0;
}
void	VAInstallQDSearch()
{
	AddSearch(MyQDColorSearchProc);
}
void	VARemoveQDSearch()
{
	DelSearch(MyQDColorSearchProc);
}

void	*VAColorToQD(thecolor)
int		thecolor;
{
	VAIndexor.blue=thecolor;

	return	(void *)&VAIndexor;
}