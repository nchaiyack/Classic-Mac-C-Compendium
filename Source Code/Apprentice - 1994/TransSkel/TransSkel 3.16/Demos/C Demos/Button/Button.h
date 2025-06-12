

# define	returnKey	13
# define	enterKey	3
# define	escapeKey	27


# define	normalHilite	0
# define	dimHilite		255


# define	aboutAlrtRes	1000

# define	modal1Res		1001
# define	modal2Res		1002
# define	modal3Res		1003
# define	modelessRes		1004
# define	movableRes		1005

# define	docWindRes		1000

# define	fileMenuRes		1000


typedef enum
{
	doModal1 = 1,
	doModal2,
	doModal3,
	doMovable,
	sepLine,
	quitApp
};


/*
 * Horizontal and vertical ratios for positioning the dialogs on the screen
 * according the Apple's (current) Human Interface Guidelines.
 */

# define	horizRatio		FixRatio (1, 2)
# define	vertRatio		FixRatio (1, 5)


void AdjustMenus (void);
void DoModal1 (void);
void DoModal2 (void);
void DoModal3 (void);
void DoMovableModal (void);
void SetupDocument (void);
void SetupModeless (void);
