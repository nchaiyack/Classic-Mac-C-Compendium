/*
	MultiSkel.h - TransSkel multiple-window demonstration Header file.
	
	#defines demo resource numbers and some values used many places,
	such as nil.
*/


# ifndef	nil
# define	nil		(0L)
# endif


# define	normalHilite	0
# define	dimHilite		255

/*
	resource numbers
*/

# define	aboutAlrtRes	1000
# define	getInfoDlog		1001

# define	fileMenuRes		1001
# define	editMenuRes		1002

# define	helpWindRes		1000
# define	editWindRes		1001
# define	zoomWindRes		1002
# define	rgnWindRes		1003

# define	helpTextRes		1000


extern	WindowPtr	helpWind;
extern	WindowPtr	editWind;
extern	WindowPtr	zoomWind;
extern	WindowPtr	rgnWind;

extern	MenuHandle	editMenu;

void DrawGrowBox (WindowPtr wind);
void SetWindClip (WindowPtr wind);
void ResetWindClip (void);

void EditWindInit (void);
void HelpWindInit (void);
void RgnWindInit (void);
void ZoomWindInit (void);
pascal void EditWindEditMenu (short item);
