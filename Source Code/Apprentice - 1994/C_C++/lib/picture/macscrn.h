//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		macscrn.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	Oct. 3, 1990
*
*	Declaration of Mac Screen specific classes, to encapsulate
*	machine-specific graphics code.
*/

# ifndef	macscrn_h
# define	macscrn_h

# include	"screen.h"
# include	"Windows.h"	// TC4: # include	"WindowMgr.h"

# define	SCREEN	Mac_Screen

/******************************************************************
*	Mac_Screen class to allow graphics I/O on Macintosh computers
******************************************************************/
class	Mac_Screen:public Generic_Screen
{
private:
	WindowPtr	window[MAX_WINDOWS];
	WindowPtr	current_window;
	int			old_mbar_height;

public:
	Mac_Screen(void);
	virtual int		new_window(Frame*);
	virtual void	make_closest(int);
	virtual void	get_window_device_frame(int,Frame*);
	virtual void	set_current_window(int);
	virtual void	set_pen_color(color);
	virtual void	fill_window(void);
	virtual void	move_to(Coord2*);
	virtual void	draw_to(Coord2*);
	virtual boolean	mouse_button_is_down(void);
	virtual			~Mac_Screen(void);
};

#endif 
