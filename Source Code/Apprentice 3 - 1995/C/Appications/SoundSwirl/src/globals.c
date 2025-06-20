/*****************
** global.c
**
** location of all the global variables
******************/


MenuHandle		gFileM, gEditM,
				gAppleM, gListenM;		/* handles to menus    */
WindowPtr		gMainWindow;			/* the main window     */
int				gListening=0;			/* Are we listening to input? */


/*** circle drawing stuff ***/

#define PI 3.1415926

double gAngleStep=PI/36.0;  /* radians per step, initial=5� */
short gOffset=0;      /* offset from radius */
short gRadius=10;    /* radius size, default is 100 */
