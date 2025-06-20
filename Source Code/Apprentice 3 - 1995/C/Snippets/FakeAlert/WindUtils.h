extern Boolean	HasColorQD;
extern RGBColor		rgbblack;
extern RGBColor		rgbwhite;
extern RGBColor		rgbdarkred;
extern RGBColor		rgblitered;

void	DrawShadowBox(Rect Box, Boolean Inside);
void	SaveTextSettings(void);
void	RestoreTextSettings(void);
short	MaxTextHeight(void);
void    SetMouse(Point Where);
void	ColorImplemented(void);
void	SetWindColors(WindowPtr wind, Boolean NotInvertColors);
void	BlackNWhite(Boolean NotInvertColors);