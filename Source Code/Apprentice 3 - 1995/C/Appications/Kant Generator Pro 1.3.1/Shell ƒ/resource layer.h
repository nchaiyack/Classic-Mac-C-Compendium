#ifndef __MY_RESOURCE_LAYER_H__
#define __MY_RESOURCE_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	OSErr OpenTheResFile(FSSpec* fs, short* oldRefNum, short* newRefNum, Boolean* alreadyOpen,
	Boolean readOnly);
extern	void CloseTheResFile(short oldRefNum, short newRefNum, Boolean alreadyOpen);
extern	Handle SafeReleaseResource(Handle resHandle);
extern	Handle SafeDisposeIconSuite(Handle iconHandle);
extern	MenuHandle SafeDisposeMenu(MenuHandle theMenu);
extern	OSErr AddIndString(StringPtr theStr, short resID);
extern	OSErr DeleteIndString(short resID, short index);
extern	OSErr SetIndString(StringPtr theStr,short resID,short strIndex);

#ifdef __cplusplus
}
#endif

#endif
