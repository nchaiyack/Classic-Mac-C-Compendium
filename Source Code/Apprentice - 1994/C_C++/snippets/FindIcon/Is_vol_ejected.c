#include "Is_vol_ejected.h"

Boolean	Is_vol_ejected( short vRefNum )
{
	OSErr		err;
	HVolumeParam	vol_pb;
	
	vol_pb.ioNamePtr = NULL;
	vol_pb.ioVRefNum = vRefNum;
	vol_pb.ioVolIndex = 0;
	err = PBHGetVInfoSync( (HParmBlkPtr) &vol_pb );
	
	return (err == noErr) && (vol_pb.ioVDRefNum > 0);
}
