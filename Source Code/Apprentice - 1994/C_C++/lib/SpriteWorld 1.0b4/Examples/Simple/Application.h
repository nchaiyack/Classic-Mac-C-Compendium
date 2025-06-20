///--------------------------------------------------------------------------------------
//	Application.h
//
//	Created:	Sunday, April 11, 1993
//	By:		Tony Myles
//
//	Copyright: � 1993 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __APPLICATION__
#define __APPLICATION__


enum
{
	kNumberOfMoreMastersCalls = 3
};

enum
{
	kWindowResID = 128
};

enum
{
	kErrorAlertResID = 128,
	kCantRunAlertResID = 129,
	kErrorStringListResID = 128,
	kFatalErrorStringIndex = 1,
	kCantFindResourceStringIndex = 2
};

#define kSeriousDamageString "\pCould not even get error string!\rThis application is seriously damaged!"


#ifdef __cplusplus
extern "C" {
#endif

void main(void);
void Initialize(short numberOfMasters);
Boolean CheckSystem(void);
void CreateWindow(void);
void ErrorAlert(OSErr err, short errorStringIndex);
void FatalError(OSErr err);
void CantFindResource(void);
void CantRunOnThisMachine(void);

#ifdef __cplusplus
};
#endif


#endif /* __APPLICATION__ */