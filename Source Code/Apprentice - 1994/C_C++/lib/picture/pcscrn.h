//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		pcscrn.h
*	AUTHOR:		Norman Gaskill
*	CREATED:	Dec. 16, 1990
*
*	Declaration of PC Screen specific classes, to encapsulate
*	machine-specific graphics code.
*/

# ifndef	pcscrn_h
# define	pcscrn_h

# include	"screen.h"

# define	SCREEN	PC_Screen

/******************************************************************
*	PC_Screen for graphics output on IBM PC compatibles
******************************************************************/
class	PC_Screen:public Generic_Screen
{
public:
	PC_Screen(void);
	virtual int		new_window(Frame*);
	virtual void	make_closest(int);
	virtual void	get_window_device_frame(int,Frame*);
	virtual void	set_current_window(int);
	virtual void	set_pen_color(int); // can't use "color" in Turbo C++?
	virtual void	fill_window(void);
	virtual void	move_to(Coord2*);
	virtual void	draw_to(Coord2*);
	virtual void	wait(void);
	virtual			~PC_Screen(void);
};

#endif
