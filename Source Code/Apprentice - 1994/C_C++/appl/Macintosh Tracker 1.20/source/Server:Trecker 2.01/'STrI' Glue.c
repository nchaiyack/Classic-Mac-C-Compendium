
/* 'STrI' Glue.c - Glue for 'ISTk' 128 resource calls */

/* Copyright (c) 1991-93 by Frank Seide, Wirichsbongardstr. 43, D-52062 Aachen, Germany */

#define GoSTrIDispatcher(sel){\
	asm{pea (sel)}\
	asm{bra STrIJump}\
}

void STrIJump()
{
	extern STrIDispatcher();
	asm {
#ifdef NO_RESOURCES
		move.l		(a7)+,d0
		jmp		STrIDispatcher
#else
		move.l		#0,-(a7)
		move.l		#'ISTk',-(a7)
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

void GetSoundTrack() GoSTrIDispatcher(1)
void HGetSoundTrack() GoSTrIDispatcher (2)
void FSpGetSoundTrack() GoSTrIDispatcher (3)
void PutSoundTrack() GoSTrIDispatcher (4)
void HPutSoundTrack() GoSTrIDispatcher (5)
void FSpPutSoundTrack() GoSTrIDispatcher (6)

/*** Handling SoundTrack resources: ***/

void GetSoundTrackResource() GoSTrIDispatcher (7)
void GetNamedSoundTrackResource() GoSTrIDispatcher (8)
void AddSoundTrackResource() GoSTrIDispatcher (9)

/*** Memory management: ***/

void NewSoundTrack() GoSTrIDispatcher (10)
void DisposeSoundTrack() GoSTrIDispatcher (11)
void MoreInstruments() GoSTrIDispatcher (12)
void MoreVoices() GoSTrIDispatcher (13)
void MorePatterns() GoSTrIDispatcher (14)
void ReshapePattern() GoSTrIDispatcher (15)

/*** Playback functions: ***/

void LockSoundTrack() GoSTrIDispatcher (16)
void UnlockSoundTrack() GoSTrIDispatcher (17)

void LinkSoundTrack() GoSTrIDispatcher (18)
void UnlinkSoundTrack() GoSTrIDispatcher (19)
void UpdateSoundTrack() GoSTrIDispatcher (20)

/*** Other functions: ***/

void RecalcSoundTrack() GoSTrIDispatcher (21)
void SetSoundTrackStereoPosition() GoSTrIDispatcher (22)
void GetSoundTrackInstrument() GoSTrIDispatcher (23)
void GetSoundTrackWorkspace() GoSTrIDispatcher (24)
void GetSoundTrackSongPos() GoSTrIDispatcher (25)
void GetSoundTrackPattern() GoSTrIDispatcher (26)
void VersionSoundTrack() GoSTrIDispatcher (0)
void CopyrightSoundTrack() GoSTrIDispatcher (27)

/*** Credits: ***/

void STrICopr()		/* This string must *NOT* be removed! */
{
	asm {
		dc.b	"*** 'ISTk' resource stub V2.0, 09/07/93, "
		dc.b	"resource and stub "
		dc.b	"copyright (c) 1991-93 by Frank Seide, Wirichsbongardstr. 43, "
		dc.b	"D-52062 Aachen, Germany *** "
		dc.b	"This message must not be changed or removed."
	}
}
