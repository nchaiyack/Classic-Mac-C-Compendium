
#define iNewFolder				1
#define iFind					2
#define iFindAgain				3
#define iLine21					4
#define iPermDirectories		5
#define iPermFiles				6
#define iConfigure				7
#define iLine22					8
#define iAbout					9
#define iLine23					10
#define iTrash					11


extern Str255		gStringToFind;


short				HandleOtherMenu(DialogPtr dlg, short menuItem);
void				RebuildMenus(ControlHandle menuControl);
