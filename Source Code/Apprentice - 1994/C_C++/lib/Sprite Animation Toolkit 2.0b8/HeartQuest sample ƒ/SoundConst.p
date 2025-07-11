{=================================================}
{============= Sound Constants and handles ==============}
{=================================================}

{ Example file for Ingemars Sprite Animation Toolkit. }
{ � Ingemar Ragnemalm 1992 }
{ See doc files for legal terms for using this code. }

{ This file defines all the resource numbers for your sound resources, plus}
{ handles to each one. The procedure LoadSounds is called at startup. It calls}
{ the sound units for each sound to preload. Note that merely loading the sound}
{ is not enough if the sound units decide to use Sound Driver. }

{ When designing your game, my advice to you is to use sounds, several sounds,}
{ but not ridiculously many and large. A simplistic game that takes 1000k on disk}
{ just because it contains lots of sounds is just a waste of disk (and we have seen}
{ far too many of them on the shareware market), but a well-polished, complex}
{ and interesting game can use more. }

unit SoundConst;

interface

	uses
		SAT;

	const
		Splatt2Snd = 14315;
		Splatt3Snd = 20039;
		KlounkSnd = 21999;
		plinkSnd = 10873;
		sadarSnd = 25626;
		jaSnd = 3024;


	var
		Splatt2SndH, Splatt3SndH, KlounkSndH: handle;
		plinkSndH, sadarSndH, jaSndH, TickSndH: handle;

	procedure LoadSounds;

implementation

	procedure LoadSounds;
	begin
		Splatt2SndH := SATGetSound(Splatt2Snd);
		Splatt3SndH := SATGetSound(Splatt3Snd);
		KlounkSndH := SATGetSound(KlounkSnd);
		PlinkSndH := SATGetSound(PlinkSnd);
		SadarSndH := SATGetSound(SadarSnd);
		JaSndH := SATGetSound(JaSnd);
		TickSndH := SATGetNamedSound('tick');
{Why I'm using SATGetSound most of the time but SATGetNamedSound for the last one?}
{Because I started using resource id's, but later decided that the names are the "right"}
{way to get sounds! Use whatever way you like best.}
	end;

end.