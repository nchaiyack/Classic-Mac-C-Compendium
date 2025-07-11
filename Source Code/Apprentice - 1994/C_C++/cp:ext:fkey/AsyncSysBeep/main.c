#include <Traps.h>
#include <Sound.h>
#include "main.h"





long			gRealSysBeep;
long			gRealSystemTask;
short			gSysBeepID;
Handle			gSndHandle;
SndChannelPtr	gSndChannel;
short			gSndLevel;





void main(void)
{
	asm
	{
		move.l	A4,-(SP)			;save A4
		lea		main,A4				;setup globals
		move.l	A4,A0				;put ptr to code into A0
		dc.w	_RecoverHandle		;recover handle to my code
		move.l	A0,-(SP)			;put handle to code onto stack
		move.l	A0,-(SP)			;put handle to code onto stack
		dc.w	_DetachResource		;detach my handle to code
		move.l	(SP)+,A0			;put handle to code into A0
		dc.w	_HLock				;lock my handle to code
	}
	
	
	gSndChannel = 0L;
	gSndHandle = 0L;
	gSysBeepID = 0;
	gSndLevel = 0;
	
	gRealSysBeep = NGetTrapAddress(_SysBeep,GetTrapType(_SysBeep));
	NSetTrapAddress((long)MySysBeep,_SysBeep,GetTrapType(_SysBeep));
	gRealSystemTask = NGetTrapAddress(_SystemTask,GetTrapType(_SystemTask));
	NSetTrapAddress((long)MySystemTask,_SystemTask,GetTrapType(_SystemTask));
	
	
	asm
	{
		move.l	(SP)+,A4			;restore A4
	}
}





pascal void MySysBeep(short duration)
{
	asm
	{
		move.l	A4,-(SP)			;save A4
		lea		main,A4				;restore globals
		
		move.b	sdVolume,-(SP)		;save speaker volume
		move.b	WWExist,-(SP)		;save InitWindows flag
		clr.b	sdVolume			;set speaker volume to zero
		move.b	#0xFF,WWExist		;set InitWindows flag to non-inited
		
		move.w	duration,-(SP)		;put duration on the stack
		move.l	gRealSysBeep,A0		;put real SysBeep address in A0
		jsr		(A0)				;jump to real SysBeep for compatibility
		
		move.b	(SP)+,WWExist		;restore InitWindows flag
		move.b	(SP)+,sdVolume		;restore speaker volume
		
		move.l	ExpandMem,A0		;put address of some system global ptr into A0
		movea.l	272(A0),A0			;put address of some other system global ptr into A0
		tst.w	24(A0)				;test SysBeep enable flag
		beq.s	@end
		
		clr.w	-(SP)				;put arg for DoSysBeep on stack
		movea.l	A7,A0				;put address in A0 for result of ReadXPram
		move.l	#0x0002007C,D0		;setup ReadXPram call for SysBeep ID
		dc.w	_ReadXPRam			;read SysBeep ID from extended pram
		
		jsr		DoSysBeep			;do the beeping
		addq	#2,SP				;clean up stack
		
@end	move.l	(SP)+,A4			;restore A4
	}
}





void DoSysBeep(short beepRsrcID)
{
	SndCommand	cmd;
	
	
	if (!gSndHandle || !gSndChannel || (beepRsrcID != gSysBeepID))
		LoadNewSysBeep(beepRsrcID);
	
	if (gSndHandle&&gSndChannel)
		if (!SndPlay(gSndChannel,gSndHandle,true))
		{
			gSndLevel++;
			
			cmd.cmd = callBackCmd;
			if (SndDoCommand(gSndChannel,&cmd,false))
				gSndLevel--;
		}
}





void LoadNewSysBeep(short beepRsrcID)
{
	Byte	resLoadSave;
	THz		zoneSave;
	short	err;
	
	
	gSysBeepID = beepRsrcID;
	
	zoneSave = GetZone();
	SetZone(SystemZone());
	
	resLoadSave = GetResLoad();
	SetResLoad(true);
	
	
	JunkOldSysBeep();
	gSndLevel = 0;
	
	err = SndNewChannel(&gSndChannel,0,0,(ProcPtr)SndPlayCompletion);
	if (err)
		gSndChannel = 0L;
	else
	{
		gSndHandle = RGetResource('snd ',beepRsrcID);
		if (gSndHandle)
			HLock(gSndHandle);
	}
	
	
	SetResLoad(resLoadSave);
	SetZone(zoneSave);
}





void JunkOldSysBeep(void)
{
	if (gSndChannel)
	{
		SndDisposeChannel(gSndChannel,true);
		gSndChannel = 0L;
	}
	
	if (gSndHandle)
	{
		HUnlock(gSndHandle);
		ReleaseResource(gSndHandle);
		gSndHandle = 0L;
	}
}





pascal void SndPlayCompletion(SndChannelPtr chan,SndCommand cmd)
{
	asm
	{
		move.l	A4,-(SP)			;save A4
		lea		main,A4				;restore globals
	}
		
	
	
	if (gSndLevel)
		gSndLevel--;
	
	
	asm
	{
		move.l	(SP)+,A4			;restore A4
	}
}





pascal void MySystemTask(void)
{
	asm
	{
		clr.l	-(SP)					;reserve space for real SystemTask return address
		move.l	A4,-(SP)				;save A4
		lea		main,A4					;restore globals
		move.l	gRealSystemTask,4(A7)	;put address of real SystemTask in reserved space
	}
		
	
	if (gSndLevel == 0)
		JunkOldSysBeep();
	
	
	asm
	{
		move.l	(SP)+,A4				;restore A4
	}
}