class	SimpleWindowClass {
	WindowPtr	theWindow;
	void		DrawWindowNumber();
	Str255		sWindowNumber;
	Str255		sThisWindow;
	short		numWidth, titleWidth;
	short		fontHeight;
public:
	void	CreateWindow(short windowNumber, short resID);
	~SimpleWindowClass();
	
	void	UpdateWindow();
	void	ActivateWindow(Boolean activate);
	void	Drag(Point where);
	void	Hide() { HideWindow(theWindow); }
	
	Boolean	operator == (SimpleWindowClass right) { return *this==right; }
	Boolean	operator != (SimpleWindowClass right) { return *this!=right; }
	Boolean operator == (WindowPtr right) { return theWindow==right; }
};
