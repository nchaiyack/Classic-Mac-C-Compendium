
/* Include file for 'PSyn', "Polyphonic Channels" */

/* Copyright © 1991-1993 by Frank Seide, Hamburg */

#ifndef __PCHANNEL__
#define __PCHANNEL__

#ifndef __SOUND__
#include "sound.h"
#endif

#define STEREO_POSITIONS 64	/* Soviele Stereopositionen mšglich */

#define STEREO_LEFT 0			/* Linker Kanal, Faktor 2 */
#define STEREO_RIGHT 2			/* Rechter Kanal, Faktor 2 */
#define STEREO_CENTER 4			/* Mitte, jeweils Faktor 1 */
#define STEREO_SURROUND 6		/* Diffus, jeweils Faktor 1 */
#define STEREO_OTHER 8			/* Sonstige Position, Summe der Faktoren ist 2 */

/* Polyphonic Channel: Description of the polyphonic sound channel. */

typedef struct PChannel {

	/* Interfacing to the song interpreting routine: */

	long resLong1;			/* $00 */
	void (*userInit)();		/* $04: (Re-)Init the song interpretation */
	int (*userPlay) (struct SoundTrack * soundTrack);	/* $08: IRQ: go ahead one step */
	void (*userCompletion)(); /* $0c: IRQ: last buffer has been played */
	long resLong2;			/* $10 */
	void * userData;		/* $14: Pointer to data private to the interpreting routine */

	/* Flags: */

	Boolean playing;		/* $18: Still playing ? (RO) */
	Boolean antiAlias;		/* $19: Apply anti-aliasing filter (in real-time) ? (R/W) */
	Boolean stereo;		/* $1a: Stereo ? (RO; use StereoPChannel() to change) */
	Boolean userMixing;	/* $1b: Use Mixing Console ? (R/W) */
	Boolean noSurround;	/* $1c: Suppress generation of surround sound (R/W) */
	Boolean fadeOut;		/* $1d: Set to TRUE to trigger fade out and stop (WO) */
	
	Boolean realTime;		/* $1e: Real-time computation on sound IRQ level 2 ? (R/W) */
	Byte cycleLimit;		/* $1f */
	
	/* Sampling rates: */
	
	Fixed pitchFactor;		/* $20: Fine tuning of the pitch (0.5É2.0) (R/W) */
	Fixed hardFreq;			/* $24: Output sampling freq. (used only by StartpCh.()) */
	long unused2b;			/* $28 */
	
	/* Volume, clipping: */
	
	Fixed mainVolume;		/* $2c: Total volume (R/W) */
	int clipped;				/* $30: Number of samples that had to be clipped (RO) */
	
	/* More flags: */
	
	Boolean muting;		/* $32: Muting (TRUE => silence, computation goes on, R/W) */
	Boolean calcPower;	/* $33: Compute channel sig. power (Set by user, clr'd by IRQ) */
	Boolean userInited;	/* $34 */
	
	/* Support for real-time displays (e. g. oscilloscope): */
	
	Boolean ossiBufEmpty;	/* $35: ossiBuffer is empty (Set by user, clr'd by  IRQ) */
	int ossiBufSize;		/* $36: Size of ossiBuffer (0, if none) */
	Byte * ossiBuffer;		/* $38: Pointer to ossiBuffers (NULL, if none) */
	long unused4;			/* $3c */

	/* More private variables following here. */	
	/*** Do not access variables beyond this line. They are subject to change! ***/

} PChannel, *PChannelPtr;

#define pr_userInit 0x04
#define pr_userPlay 0x08
#define pr_userCompletion 0x0c
#define pr_userData 0x14
#define pr_playing 0x18
#define pr_antiAlias 0x19
#define pr_stereo 0x1a
#define pr_userMixing 0x1b
#define pr_noSurround 0x1c
#define pr_fadeOut 0x1d
#define pr_realTime 0x1e
#define pr_cycleLimit 0x1f
#define pr_pitchFactor 0x20
#define pr_hardFreq 0x24
#define pr_mainVolume 0x2c
#define pr_clipped 0x30
#define pr_muting 0x32
#define pr_calcPower 0x33
#define pr_userInited 0x34
#define pr_ossiBufEmpty 0x35
#define pr_ossiBufSize 0x36
#define pr_ossiBuffer 0x38

/* OpenPChannel() flags: */
#define opc_stereo 0x01			/* Open stereo channel */
#define opc_antiAlias 0x02		/* Apply anti-aliasing filter */
#define opc_perfect 0x04			/* Use real low pass filter instead of simple */
									/* linear interpolation. */
									/* The low pass is designed to suppress aliasing */
									/* spectra below quantization error. Therefore */
									/* at 16 bit a much longer low pass is used */
									/* than at 8 bit. */
									/* The perfect filtering requires much more CPU */
									/* time. */
#define opc_16Bit 0x40			/* Open 16 bit channel */
#define opc_quiet 0x80			/* Open channel without SndChannel */

/* Function prototypes: */

extern pascal Boolean CheckPChannel (struct PChannel * pc);
extern pascal void StopPChannel (struct PChannel * pc, Boolean fadeOut);
extern pascal void ClosePChannel (struct PChannel * pc);
extern pascal void ResetPChannel (struct PChannel * pc);
extern pascal OSErr OpenPChannel (int channels, Byte flags, int playFrames, struct PChannel ** pcp);
extern pascal OSErr StartPChannel (struct PChannel * pc);
extern pascal OSErr StereoPChannel (register struct PChannel * pc, Boolean stereo);
extern pascal void PChannelVolume (struct PChannel * pc, int chan, Fixed volume);
extern pascal int PChannelVersion();
extern pascal void PChannelPosition (struct PChannel * pc, int chan, int stereoPosition);
extern pascal unsigned int GetPChannelSignalPower (struct PChannel * pc, unsigned int chan);
extern pascal long ReadPChannel (PChannelPtr pc, unsigned long bufBytes, void * buf);
extern pascal void PChannelCopyright (StringPtr s);

#endif __PCHANNEL__