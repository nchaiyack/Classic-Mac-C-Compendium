//-------------------------------------------------------------------------------
// UAEFinder.h			Originally by Peter N Lewis (peter@mail.peter.com.au)
//						Converted to C++ by Gilles Dignard (gdignard@hookup.net)
//-------------------------------------------------------------------------------
#pragma once

const OSType application = 'APPL';

class UAEFinder
{
public:
	static Boolean	TellFinderToLaunch	(FSSpec&	ioSpec,
										 Boolean	inToFront);
	
	static Boolean	OpenControlPanel	(OSType		inCreator);
	
	static void		LaunchWithDocument	(OSType		inCreator,
										 OSType		inType,
										 FSSpec&	inSpec,
										 Boolean	inToFront);
	
	static void		LaunchApp			(OSType		inCreator,
										 OSType		inType,
										 Boolean	inToFront);
	
	static void		LaunchFSSpec		(FSSpec&	ioSpec,
										 Boolean	inToFront);
	
	static void		QuitApplication		(OSType		inCreator,
										 OSType		inType);
	

	//
	// Other useful procedures. Not strictly AE, but ones used
	// by the AE routines.
	//
	static OSErr	FindApplication 	(OSType		inCreator,
										 FSSpec&	ioSpec);

	static Boolean	FindProcess			(OSType					inCreator,
										 OSType					inType,
										 ProcessSerialNumber&	ioProcess,
										 FSSpec&				ioSpec);

	static OSErr	FindControlPanel	(OSType		inCreator,
										 FSSpec&	ioSpec);


private:
	static Boolean	BTST				(long			inLong,
										 short			inBitNo);
	
	static void		PrepareToLaunch		(AppleEvent&			ioEvent,
										 Boolean				toFront,
										 AEDesc&				ioLaunchDesc,
										 LaunchParamBlockRec&	ioLaunchThis);

	static void		AddFSSToAEList		(AEDescList&	ioList,
										 short			inRow,
										 FSSpec&		ioSpec);
};
