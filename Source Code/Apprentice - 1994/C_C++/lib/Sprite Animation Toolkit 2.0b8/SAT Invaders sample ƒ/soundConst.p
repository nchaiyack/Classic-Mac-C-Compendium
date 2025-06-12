{=================================================}
{============= Sound Constants and handles ==============}
{=================================================}

{ Example file for Ingemars Sprite Animation Toolkit. }
{ © Ingemar Ragnemalm 1992 }
{ See doc files for legal terms for using this code. }

{ This file defines handles for your sound resources. The procedure LoadSounds}
{ is called at startup. The sounds could also have been loaded by the appropriate}
{ sprite units.}

{ When designing your game, my advice to you is to use sounds, several sounds,}
{ but not ridiculously many and large. A simplistic game that takes 1000k on disk}
{ just because it contains lots of sounds is just a waste of disk (and we have seen}
{ far too many of them on the shareware market), but a well-polished, complex}
{ and interesting game can and should use more. }

unit SoundConst;

interface

	uses
		SAT;

	var
		toffH, dunkH, piuH, kraschH: handle;

	procedure LoadSounds;

implementation

	procedure LoadSounds;
	begin
		toffH := SATGetNamedSound('toff');
		dunkH := SATGetNamedSound('Dunk');
		piuH := SATGetNamedSound('piu');
		kraschH := SATGetNamedSound('Krasch 1');
	end;

end.