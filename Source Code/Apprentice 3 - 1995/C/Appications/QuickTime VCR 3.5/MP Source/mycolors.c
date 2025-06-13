#include "Alert_Class.h"
#include "mycolors.h"




void SetColor( int index )
{
	RGBColor	color;
	
	switch (index) {
		case sl_black:
			color.red = color.green = color.blue = 0;
			break;
		case sl_white:
			color.red = color.green = color.blue = 0xffff;
			break;
		case sl_darkdarkGray:
			color.red = color.green = color.blue = 0x1999;
			break;
		case sl_darkGray:
			color.red = color.green = color.blue = 0x3fff;
			break;
		case sl_gray:
			color.red = color.green = color.blue = 0x7fff;
			break;
		case sl_liteGray:
			color.red = color.green = color.blue = 0xbfff;
			break;
		default : break;
	}
	
	
	RGBBackColor( &color );
	RGBForeColor( &color );	
}

/*-------------------------------------------------------------------------------------*/
