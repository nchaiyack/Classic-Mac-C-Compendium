/* disk.c */

#include <Files.h>
#include <OSUtils.h>
#include <stdio.h>

void EjectDisk();
Boolean IsDisk( void );

void EjectDisk( void )
{
	Str255		volName;
	short		vRefNum;
	long		freeBytes;
	OSErr		err;
	
	err = GetVInfo( 1, volName, &vRefNum, &freeBytes );
	err = UnmountVol( 0, vRefNum );
		
	err = GetVInfo( 1, volName, &vRefNum, &freeBytes );
	err = Eject( 0, vRefNum );
}
 
Boolean IsDisk( void )
{
	QHdrPtr	qhp;			/* address of a QHdr structure */
	DrvQEl	*qep;
	Byte	*bp;				/* helps to decode prefix bytes */
	long	totBlks;
	Boolean	locked, oneSide, empty;
	
	qhp= GetDrvQHdr();			/* address of queue header */
	qep = (DrvQEl *)qhp->qHead;	/* address of a queue element */
	
		bp=(Byte *)qep;  bp -=4;	 /* point to structure prefix bytes */
		if ( qep->qType == 0 )
			totBlks = qep->dQDrvSz;	/* get size (in logical blocks) */
		else
			totBlks = qep->dQDrvSz + (((long) qep->dQDrvSz2) << 16);
	
		locked = oneSide = empty = FALSE;	/* set Booleans from prefix flags */
//		if ( (bp[0] & 0x80)==0x80 ) locked=TRUE;
//		if ( (bp[3] & 0x80)==0 ) oneSide=TRUE;
		if ( bp[1]==0 ) empty=TRUE;

	return( !empty );
}