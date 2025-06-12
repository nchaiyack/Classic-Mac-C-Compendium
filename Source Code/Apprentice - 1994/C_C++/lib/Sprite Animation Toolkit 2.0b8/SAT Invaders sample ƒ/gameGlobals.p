{=================================================}
{========= GameGlobals, globals for SATInvaders ==========}
{=================================================}

{ Example file for Ingemars Sprite Animation Toolkit. }
{ © Ingemar Ragnemalm 1992 }
{ See doc files for legal terms for using this code. }

{ This file defines the resource numbers and global variables for SATInvaders.}

unit GameGlobals;

interface

{uses SAT;}

	const
{	Resource numbers}

		fileMenuRes = 1004;	{ Race menu }
		aboutAlrt = 1000;		{ About box }
		gameWindRes = 262;		{ game window }

{ File menu }
		run = 1;
		sound = 3;
		fastAnimation = 4;
		quit = 6;

	var
{	Menu handles.  There isn't any apple menu here, since TransSkel will}
{	be told to handle it itself.}

		fileMenu: MenuHandle;

{Level number.}
		level: longint;

implementation

end.