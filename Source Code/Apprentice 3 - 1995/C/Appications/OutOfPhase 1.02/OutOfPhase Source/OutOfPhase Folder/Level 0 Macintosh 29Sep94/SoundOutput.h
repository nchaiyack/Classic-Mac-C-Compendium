/* SoundOutput.h */

#ifndef Included_SoundOutput_h
#define Included_SoundOutput_h

/* SoundOutput module depends on */
/* MiscInfo.h */
/* Definitions */
/* Debug */
/* Audit */
/* Memory */

typedef enum {eStereo EXECUTE(= -28741), eMono} SoundOutputStereo;

typedef enum {e8bit EXECUTE(= -14274), e16bit} SoundOutputNumBits;

/* attempt to obtain a sound channel.  returns True if the sound channel was opened */
/* or False if it couldn't be (if already in use or machine doesn't support sound) */
MyBoolean		OpenSoundChannel(long SamplingRate, SoundOutputStereo WantStereo,
							SoundOutputNumBits NumBits, long FramesPerBuffer, int MaxNumBuffers,
							int InitialNumBuffers);

/* close the sound channel and clean up the buffers */
void				CloseSoundChannel(void (*Callback)(void* Refcon), void* Refcon);

/* obtain a pointer to one of the [nonrelocatable] sound buffers.  Data in this */
/* buffer is interpreted as such:  For 8-bit mono, the buffer is an array of */
/* signed chars.  For 16bit mono, the buffer is an array of short signed integers in */
/* the machine's native endianness.  For 8-bit stereo, the buffer is an array of */
/* 2-byte tuples; the byte lower in memory is the left channel.  For 16-bit stereo, */
/* the buffer is an array of 2-short) tuples, the left channel is lower in memory. */
/* If there are no buffers currently available (and new ones couldn't be allocated) */
/* then it returns NIL */
char*				CheckOutSoundBuffer(void);

/* submit a buffer to be queued to the system's sound channel.  The number of frames */
/* in the buffer actually used is specified to allow less than the full buffer to */
/* be used.  If the buffer is full, then the callback is repeatedly called. */
void				SubmitBuffer(char* Buffer, long NumUsedFrames,
							void (*Callback)(void* Refcon), void* Refcon);

/* discard all queued data on the sound channel and close it immediately */
void				KillSoundChannel(void);

#endif
