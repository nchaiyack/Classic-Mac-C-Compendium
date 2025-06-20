#include "CEToolbox.h"

short				gCERefNum;

//--------------------------------------------------------------------------------

OSErr	CEOpenToolbox()
{
	OSErr				err;
	ParamBlockRec		pb;

	gCERefNum = 0;
	pb.ioParam.ioNamePtr = "\p.Hot";
	pb.ioParam.ioPermssn = 0;
	err = PBOpenSync(&pb);
	if (err == noErr) {
		gCERefNum = pb.ioParam.ioRefNum;
	}

	return err;
}


//--------------------------------------------------------------------------------

OSErr	CERegister(HBQRec* record)
{
	ParamBlockRec		pb;

	pb.cntrlParam.ioCRefNum = gCERefNum;
	pb.cntrlParam.csCode = DoRegisterPtr;
	*(HBQPtr *) &pb.cntrlParam.csParam[0] = record;
	return PBControlSync(&pb);
}


//--------------------------------------------------------------------------------

OSErr	CEPostError(short errNum)
{
	ParamBlockRec		pb;

	pb.cntrlParam.ioCRefNum = gCERefNum;
	pb.cntrlParam.csCode = DoPostError;
	pb.cntrlParam.csParam[0] = errNum;
	return PBControlSync(&pb);
}


//--------------------------------------------------------------------------------

OSErr	CEFindApp(OSType creator, StringPtr name, short* vRefNum, long* dirID)
{
	OSErr				err;
	ParamBlockRec		pb;

	pb.cntrlParam.ioCRefNum = gCERefNum;
	pb.cntrlParam.csCode = DoFindApp;
	*(OSType    *) &pb.cntrlParam.csParam[0] = creator;
	*(StringPtr *) &pb.cntrlParam.csParam[2] = name;
	                pb.cntrlParam.csParam[6] = *vRefNum;
	err = PBControlSync(&pb);
	if (err == noErr) {
		*vRefNum = pb.cntrlParam.csParam[6];
		*dirID = *(long *) &pb.cntrlParam.csParam[4];
	}

	return err;
}


//--------------------------------------------------------------------------------

OSErr	CETheLaunch(LaunchHdl launchHandle)
{
	ParamBlockRec		pb;

	pb.cntrlParam.ioCRefNum = gCERefNum;
	pb.cntrlParam.csCode = DoTheLaunch;
	*(LaunchHdl *) &pb.cntrlParam.csParam[0] = launchHandle;
	return PBControlSync(&pb);
}


//--------------------------------------------------------------------------------

OSErr	CETurnOnOff(short newValue, short* oldValue)
{
	OSErr				err;
	ParamBlockRec		pb;

	pb.cntrlParam.ioCRefNum = gCERefNum;
	pb.cntrlParam.csCode = TurnOnOff;
	pb.cntrlParam.csParam[0] = newValue;
	err = PBControlSync(&pb);

	if (err == noErr)
		*oldValue = pb.cntrlParam.csParam[0];

	return err;
}


//--------------------------------------------------------------------------------

OSErr	CECheckKey(long key, HBQPtr* otherGuy)
{
	OSErr				err;
	ParamBlockRec		pb;

	pb.cntrlParam.ioCRefNum = gCERefNum;
	pb.cntrlParam.csCode = DoCheckKey;
	*(long *) &pb.cntrlParam.csParam[0] = key;
	err = PBControlSync(&pb);
	if (err == noErr) {
		*otherGuy = *(HBQPtr *) &pb.cntrlParam.csParam[0];
	}

	return err;
}


//--------------------------------------------------------------------------------

OSErr	CEDrawKey(long key, Rect* bounds)
{
	ParamBlockRec		pb;

	pb.cntrlParam.ioCRefNum = gCERefNum;
	pb.cntrlParam.csCode = DoDrawKey;
	*(long *) &pb.cntrlParam.csParam[0] = key;
	*(Rect* *) &pb.cntrlParam.csParam[2] = bounds;
	return PBControlSync(&pb);
}
