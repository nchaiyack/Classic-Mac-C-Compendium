//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		color.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	Oct. 6, 1990
*
*	color type; colors available.
*/

# ifndef	color_h
# define	color_h

 # define	COLOR

# ifdef		COLOR

# define	BLACK		0
# define	WHITE		1
# define	RED			2
# define	YELLOW		3
# define	GREEN		4
# define	BLUE		5
# define	CYAN		6
# define	MAGENTA		7

# else

# define	BLACK		0
# define	WHITE		1
# define	RED			0
# define	YELLOW		1
# define	GREEN		0
# define	BLUE		0
# define	CYAN		1
# define	MAGENTA		1

# endif

typedef	int	color;

# endif
