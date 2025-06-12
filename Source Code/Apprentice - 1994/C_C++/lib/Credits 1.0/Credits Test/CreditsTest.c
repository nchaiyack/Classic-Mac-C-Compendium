/*******************************************************
 *                                                     *
 *              	   Credits                         *
 *                                                     *
 *    	by Bill Hayden & Nikol Software, © 1994  	   *  
 *                                                     *
 * 													   *
 *                                                     *
 *      Requires System 7 and 256 colors to run        *
 *                                                     *
 *      Thank you for supporting Nikol Software        *
 *                                                     *
 *                                                     *
 *     Orders, support, inquiries, suggestions to:     *
 *                                                     *
 *                   Bill Hayden                       *
 *                c/o Nikol Software                   *
 *                924 Gondolier Blvd.                  *
 *            Gulf Breeze, FL  32561-3018              *
 *                					                   *
 ******************************************************/



#include "Credits.h"


/**********************************************************************************/
/*   							 .function prototypes						  	  */
/**********************************************************************************/

void	ToolBoxInit (void);


/**********************************************************************************/


void ToolBoxInit (void)
{
	MaxApplZone();
	MoreMasters();
	InitGraf( &thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil);
	InitCursor();
	FlushEvents(everyEvent, 0);
}



/**********************************************************************************/


main ()
{	
	ToolBoxInit();
	if (!ShowCredits(128, 1973, 1, kNone, 2, TRUE))
		SysBeep(10);
}
