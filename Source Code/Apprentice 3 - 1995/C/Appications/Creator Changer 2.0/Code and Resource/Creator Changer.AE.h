#include "AppleEvents.h"

pascal	OSErr	Open_AE( AppleEvent *, AppleEvent *, long);
pascal	OSErr	Quit_AE( AppleEvent *, AppleEvent *, long);
pascal	OSErr	Print_AE(AppleEvent *, AppleEvent *, long);
pascal	OSErr	Start_AE(AppleEvent *, AppleEvent *, long);

short			Multiple_Files;
long			Num_Of_Files;

extern	Boolean	All_Done;
extern	FInfo	File_Info;
extern	FSSpec	The_File_Spec;
