/* DrawPrintf.c
This does a sprintf and draws the characters into the current port, 
using the current font etc. 
*/
#include "VideoToolbox.h"
#include <stdarg.h>      /* for variable-number-of-argument macros */

void DrawPrintf(char *s, ...)
{
	va_list args;
	char string[400];
	int i;
	Point pt;
	FontInfo info;
  
	va_start(args, s);
	vsprintf(string,s,args);
	va_end(args);
	for(i=0;i<strlen(string);i++){
		if(string[i]=='\n'){
			GetPen(&pt);
			GetFontInfo(&info);
			Move(-pt.h,info.leading+info.ascent+info.descent);
		}
		else DrawChar(string[i]);
	}
}
