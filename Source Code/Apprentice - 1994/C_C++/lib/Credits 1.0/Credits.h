/*******************************************************
 *                                                     *
 *              	  Credits.h                        *
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
 *                  Nikol Software                     *
 *                  c/o Bill Hayden                    *
 *                924 Gondolier Blvd.                  *
 *            Gulf Breeze, FL  32561-3018              *
 *                					                   *
 ******************************************************/


/*	ShowCredits will return FALSE if illegal or incorrect values were passed. 		*/

pascal Boolean	ShowCredits	(short,		/* Resource ID of the Dialog				*/
							 short,		/* Resource ID of the PICT to scroll		*/
							 short,		/* DITL item # of the scrolling (user) item	*/
							 short,		/* DITL item # of the OK button, or kNone	*/
							 short,		/* Delay between frames in 1/60 secs		*/
							 Boolean);	/* Frame the scrolling item?				*/
							 
/*
	Note: there is a huge performance hit if the user item in the dialog is not the
	same width as the PICT, as CopyBits will try to scale it.  This is probably not
	what you want at all.
*/
							 
#define kNone (-1)