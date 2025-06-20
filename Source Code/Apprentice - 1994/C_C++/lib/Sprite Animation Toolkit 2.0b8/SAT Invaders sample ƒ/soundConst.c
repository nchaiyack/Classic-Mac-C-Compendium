//� C translation from Pascal source file: soundConst.p

//� =================================================.
//� ============= Sound Constants and handles ==============.
//� =================================================.

//� Example file for Ingemars Sprite Animation Toolkit.
//� � Ingemar Ragnemalm 1992.
//� See doc files for legal terms for using this code.

//� This file defines handles for your sound resources. The procedure LoadSounds.
//� is called at startup. It calls the sound units for each sound to preload. Note that.
//� merely loading the sound is not enough if the sound unit decides to use Sound Driver.

//� When designing your game, my advice to you is to use sounds, several sounds,.
//� but not ridiculously many and large. A simplistic game that takes 1000k on disk.
//� just because it contains lots of sounds is just a waste of disk (and we have seen.
//� far too many of them on the shareware market), but a well-polished, complex.
//� and interesting game can and should use more.

//� SoundConst; I know, bad name for the unit, nowadays.

//� Prototypes, etc.

#include "SAT.h"

Handle toffH, dunkH, piuH, kraschH;

void LoadSounds ()
{
	toffH = SATGetNamedSound ("\ptoff");
	dunkH = SATGetNamedSound ("\pDunk");
	piuH = SATGetNamedSound ("\ppiu");
	kraschH = SATGetNamedSound ("\pKrasch 1");
}

