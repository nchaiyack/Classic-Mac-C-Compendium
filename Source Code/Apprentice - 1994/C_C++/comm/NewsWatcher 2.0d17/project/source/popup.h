#define	kCurrentPopupItem	0

void GetPopupCString (ControlHandle ctl, short item, char *str);
void GetPopupPString (ControlHandle ctl, short item, Str255 string);
void SetPopupItemStyle (ControlHandle ctl, short item, short style);
void AddPopupItem (ControlHandle ctl, short after, Str255 str);
void DelPopupItem (ControlHandle ctl, short item);
short SetPopupValue (ControlHandle ctl, Str255 str, Boolean isNumber);
short TrackPopup (ControlHandle ctl, Point where, Str255 checkItem, Boolean isNumber);
