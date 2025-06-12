
/* 'Psyn' Glue.c - Glue for 'PSTk' 128 resource calls */

/* Copyright (c) 1991-93 by Frank Seide, Wirichsbongardstr. 43, D-52062 Aachen, Germany */

#define GoPSynDispatcher(sel){\
	asm{pea (sel)}\
	asm{bra PSynJump}\
}

void PSynJump()
{
	extern PSynDispatcher();
	asm {
#ifdef NO_RESOURCES
		move.l		(a7)+,d0
		jmp		PSynDispatcher
#else
		move.l		#0,-(a7)
		move.l		#'PSTk',-(a7)
		move.w		#128,-(a7)
		_GetResource
		move.l		(a7)+,a0
		move.l		a0,d0
		bne			@OK
		move.w		#25,d0
		_SysError
@OK	move.l		(a0),a0
		move.l		(a7)+,d0
		jmp		(a0)
#endif
	}
}

/*** Functions: ***/

void PChannelVersion() GoPSynDispatcher(0)
void CheckPChannel() GoPSynDispatcher(1)
void StopPChannel() GoPSynDispatcher(2)
void ClosePChannel() GoPSynDispatcher(3)
void ResetPChannel() GoPSynDispatcher(4)
void OpenPChannel() GoPSynDispatcher(5)
void StartPChannel() GoPSynDispatcher(6)
void StereoPChannel() GoPSynDispatcher(7)
void PChannelVolume() GoPSynDispatcher(8)
void PChannelPosition() GoPSynDispatcher(9)
void GetPChannelRecord() GoPSynDispatcher(10)
void GetPChannelSignalPower() GoPSynDispatcher(11)
void ReadPChannel() GoPSynDispatcher(12)
void PChannelCopyright() GoPSynDispatcher(13)
void PChannelBits() GoPSynDispatcher(14)

/*** Credits: ***/

void PSynCopr()		/* This string must *NOT* be removed! */
{
	asm {
		dc.b	"*** 'PSTk' resource stub V2.0.1, 12/05/93, "
		dc.b	"resource and stub "
		dc.b	"copyright (c) 1991-93 by Frank Seide, Wirichsbongardstr. 43, "
		dc.b	"D-52062 Aachen, Germany *** "
		dc.b	"This message must not be changed or removed."
	}
}
