/* CreateTrialSnds.c
Create snd resources for 2afc testing: a "beep" for each presentation,
rightSnd, wrongSnd, and abortSnd for feedback, and a "purr" that I use between 
the presentation and response to indicate to the user that the computer 
is waiting for a response. You can play these sounds by using Apple's SndPlay(), 
or my SndPlay1(), which is in the VideoToolbox. Note that these are proper
resources and are interchangeable with other snd resources, which you can
retrieve by calling GetNamedResource('snd ',"\pSimple Beep"), for example.
You could also use the sound recording capabilities of your Mac to record
spoken words like "right" and "wrong", accessible in the same way.

It's ok to call this routine repeatedly, after the first time it has no effect
since it's smart enough to notice that the sounds have already been created.

Unlike other routines in VideoToolboxSources, you will probably not use this
routine unaltered. Instead you'll probably modify the code to produce
sounds that suit your own purposes.

Note that the sound handles are published as globals. I don't like using globals,
because they make information flow impossible to work out, but in this case
all alternatives seem unreasonably cumbersome.

In updating my software from the Sound Driver to the Sound Manager I was
disappointed to discover that the squareWaveSynth supplied by the Sound
Manager doesn't sound at all like the square wave synthesizer of the
Sound Driver. It seems to interpolate, resulting in smoother waveforms that
are inaudible at low frequencies, even if you set the timbre parameter to 255.
E.g. my "purr" is a 22 Hz square wave of which one only hears the clicks at
the transitions, and is thus inaudible after smoothing. Fortunately one can 
recreate the old square wave synthesizer by using the sampledSynth with 
no interpolation, playing from a square wave table, which can be included in the
snd resource. 

Note that the beep, which uses the squareWaveSynth, continues playing even while 
interrupts are suspended, e.g. by calling SetPriority(7), while the sounds
produced by the sampledSynth pause while interrupts are suspended. This
implies that the sound chip must implement the squareWaveSynth all by itself.
However, the beep never ends as long as the interrupts remain suspended.
The practical consequence of all this is that you can use "beep" to play
sounds simultaneously with stimuli that (for maximum performance) require
suspension of interrupts.
HISTORY:
4/1/92	dgp extracted it from DoATrial.c
*/
#include "VideoToolbox.h"
#include <Sound.h>

Handle beep=NULL,rightSnd=NULL,wrongSnd=NULL,abortSnd=NULL,purr=NULL;

typedef struct {			// An 'snd ' resource of type 1. See Inside Mac VI:22-19.
	short format;			// must equal 1
	short numberOfSynths;	// 1 in this structure
	short synth;
	long synthInitOption;
	short numberOfCommands;
	SndCommand commands[];
} Snd1Resource;

void CreateTrialSnds(void)
{
	Snd1Resource *s;
	OSErr error;
	SoundHeader **squareWaveTable;
	
	if(beep!=NULL)return;
	
	squareWaveTable=(SoundHeader **)NewHandle(sizeof(SoundHeader)+8);
	if(squareWaveTable==NULL)goto error;
	(*squareWaveTable)->samplePtr=NULL;
	(*squareWaveTable)->length=8;
	(*squareWaveTable)->sampleRate=880*0x10000;	// 880 Hz
	(*squareWaveTable)->loopStart=0;
	(*squareWaveTable)->loopEnd=8;
	(*squareWaveTable)->encode=stdSH;
	(*squareWaveTable)->baseFrequency=57;		// 440 Hz
	(*squareWaveTable)->sampleArea[0]=0;
	(*squareWaveTable)->sampleArea[1]=255;
	(*squareWaveTable)->sampleArea[2]=0;
	(*squareWaveTable)->sampleArea[3]=255;
	(*squareWaveTable)->sampleArea[4]=0;
	(*squareWaveTable)->sampleArea[5]=255;
	(*squareWaveTable)->sampleArea[6]=0;
	(*squareWaveTable)->sampleArea[7]=255;

	purr=NewHandle(sizeof(Snd1Resource)+4*sizeof(SndCommand));
	if(purr==NULL)goto error;
	s=(Snd1Resource *)*purr;
	s->format=1;
	s->numberOfSynths=1;
	s->synth=sampledSynth;
	s->synthInitOption=initNoInterp;
	s->numberOfCommands=4;
	s->commands[0].cmd=soundCmd+dataOffsetFlag;
	s->commands[0].param1=0;
	s->commands[0].param2=GetHandleSize(purr);
	s->commands[1].cmd=ampCmd;
	s->commands[1].param1=70;
	s->commands[1].param2=0;
	s->commands[2].cmd=freqCmd;
	s->commands[2].param1=0;
	s->commands[2].param2=1;		// 17 Hz
	s->commands[3].cmd=nullCmd;
	s->commands[3].param1=0;
	s->commands[3].param2=0;
	HLock((Handle)squareWaveTable);
	HandAndHand((Handle)squareWaveTable,purr);	// append squareWaveTable
	DisposeHandle((Handle)squareWaveTable);
	
	abortSnd=purr;
	if(HandToHand(&abortSnd))goto error;
	s=(Snd1Resource *)*abortSnd;
	s->commands[1].cmd=ampCmd;
	s->commands[1].param1=80;
	s->commands[1].param2=0;
	s->commands[2].cmd=freqDurationCmd;
	s->commands[2].param1=2400;
	s->commands[2].param2=69-5*12;	// 27.5 Hz
	s->commands[3].cmd=quietCmd;
	s->commands[3].param1=0;
	s->commands[3].param2=0;

	wrongSnd=abortSnd;
	if(HandToHand(&wrongSnd))goto error;
	s=(Snd1Resource *)*wrongSnd;
	s->synthInitOption=0;
	s->commands[1].cmd=ampCmd;
	s->commands[1].param1=100;
	s->commands[1].param2=0;
	s->commands[2].cmd=freqDurationCmd;
	s->commands[2].param1=1200;
	s->commands[2].param2=69-12;	// 880/2 Hz

	beep=NewHandle(sizeof(Snd1Resource)+4*sizeof(SndCommand));
	if(beep==NULL)goto error;
	s=(Snd1Resource *)*beep;
	s->format=1;
	s->numberOfSynths=1;
	s->synth=squareWaveSynth;
	s->synthInitOption=0;
	s->numberOfCommands=4;
	s->commands[0].cmd=timbreCmd;
	s->commands[0].param1=20;
	s->commands[0].param2=0;
	s->commands[1].cmd=ampCmd;
	s->commands[1].param1=100;
	s->commands[1].param2=0;
	s->commands[2].cmd=freqDurationCmd;
	s->commands[2].param1=4000;
	s->commands[2].param2=69;		// 880 Hz
	s->commands[3].cmd=quietCmd;
	s->commands[3].param1=0;
	s->commands[3].param2=0;
	
	rightSnd=beep;
	if(HandToHand(&rightSnd))goto error;
	s=(Snd1Resource *)*rightSnd;
	s->commands[0].cmd=timbreCmd;
	s->commands[0].param1=0;
	s->commands[0].param2=0;
	s->commands[1].cmd=ampCmd;
	s->commands[1].param1=100;
	s->commands[1].param2=0;
	s->commands[2].cmd=freqDurationCmd;
	s->commands[2].param1=100;
	s->commands[2].param2=69+12;	// 2*880 Hz

	return;
	
	error:
	PrintfExit("CreateTrialSnds: not enough memory\n");
}
