#define		GOOD_ICON				130
#define		BAD_ICON				129

void ShowIconFamily(short);
void GetIconRect(register Rect* iconRect);
Handle ChooseIcon(short iconId, short* suggestedDepth);

#define	ShowBadICON()	ShowIconFamily(BAD_ICON)
#define	ShowGoodICON()	ShowIconFamily(GOOD_ICON)
