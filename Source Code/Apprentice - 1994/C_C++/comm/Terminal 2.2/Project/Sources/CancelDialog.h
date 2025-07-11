/*
	Terminal 2.2
	"CancelDialog.h"
*/

Boolean CheckCancel (void);
void DrawCancelDialog (short, Byte *);
void DrawProgressDialog (short, Byte *);
void InfoProgress (Byte *);
void NameProgress (Byte *);
void RemoveCancelDialog (void);
Byte *SecondsToString (long, Byte *);
void SelectCancelDialog (void);
void Statistics (long, long, short);
void UpdateProgress (long, long, long, long, long, Byte *);
