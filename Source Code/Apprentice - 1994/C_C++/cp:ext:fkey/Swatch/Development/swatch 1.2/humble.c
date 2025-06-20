#include <stdio.h>
#include <types.h>

#include <DebugPrint.h>
#include <pstring.h>

main()
{
	CInfoPBRec orig_cat_info_PB, new_cat_info_PB;
	HParamBlockRec write_PB;
	register Boolean previous_exists;
	Handle snd;
	static char riff_file_name[] = "\pSwatch Riff";

	snd = GetNamedResource( 'snd ', "\pBoingo" );
	if ( !snd )
		return;
	HLock( snd );

	write_PB.fileParam.ioNamePtr = (StringPtr) riff_file_name;
	write_PB.fileParam.ioVRefNum = 0;
	write_PB.fileParam.ioFVersNum = 0;
	PBCreate( &write_PB, FALSE );

	write_PB.ioParam.ioVersNum = 0;
	write_PB.ioParam.ioPermssn = 0;
	write_PB.ioParam.ioMisc = NULL;
	write_PB.ioParam.ioRefNum = 0;
	if ( PBOpen( &write_PB, FALSE ) )
		goto cant_save;

	write_PB.ioParam.ioBuffer = (Ptr) StripAddress( *snd );
	write_PB.ioParam.ioReqCount = GetHandleSize( snd );
	write_PB.ioParam.ioPosMode = fsFromStart;
	write_PB.ioParam.ioPosOffset = 0;
	if ( PBWrite( &write_PB, FALSE ) )
		goto cant_save;

	PBClose( &write_PB, FALSE );
	return;

cant_save:
	if ( write_PB.ioParam.ioRefNum )
		PBClose( &write_PB, FALSE );
}
