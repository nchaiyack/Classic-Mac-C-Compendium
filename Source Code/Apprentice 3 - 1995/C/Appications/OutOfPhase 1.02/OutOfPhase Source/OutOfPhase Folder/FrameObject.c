/* FrameObject.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "FrameObject.h"
#include "NoteObject.h"
#include "Memory.h"
#include "DataMunging.h"
#include "NoteImages.h"
#include "Frequency.h"
#include "StaffCalibration.h"
#include "Fractions.h"


struct FrameObjectRec
	{
		NoteObjectRec**		NoteArray; /* too much overhead to use ArrayRec object */
		OrdType						Width; /* cached width value; 0 == unknown */
	};


/* allocate a new frame. */
FrameObjectRec*				NewFrame(void)
	{
		FrameObjectRec*			Frame;

		Frame = (FrameObjectRec*)AllocPtrCanFail(sizeof(FrameObjectRec),"FrameObjectRec");
		if (Frame == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Frame->NoteArray = (NoteObjectRec**)AllocPtrCanFail(0,"FrameNoteObjectArray");
		if (Frame->NoteArray == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Frame);
				goto FailurePoint1;
			}
		Frame->Width = 0;
		return Frame;
	}


/* dispose of a frame and all notes it contains.  be careful here since this won't */
/* worry about tie references to notes contained in the frame. */
void									DisposeFrameAndContents(FrameObjectRec* Frame)
	{
		long								Scan;

		Scan = NumNotesInFrame(Frame) - 1;
		while (Scan >= 0)
			{
				DisposeNote(Frame->NoteArray[Scan]);
				Scan -= 1;
			}
		ReleasePtr((char*)(Frame->NoteArray));
		ReleasePtr((char*)Frame);
	}


/* get the number of notes contained in the frame */
long									NumNotesInFrame(FrameObjectRec* Frame)
	{
		CheckPtrExistence(Frame);
		return PtrSize((char*)(Frame->NoteArray)) / sizeof(NoteObjectRec*);
	}


/* get the note specified by the index from the frame object */
struct NoteObjectRec*	GetNoteFromFrame(FrameObjectRec* Frame, long Index)
	{
		CheckPtrExistence(Frame);
		PRNGCHK(Frame->NoteArray,&(Frame->NoteArray[Index]),sizeof(NoteObjectRec*));
		return Frame->NoteArray[Index];
	}


/* delete a note in the frame.  returns True if successful. */
MyBoolean							DeleteNoteFromFrame(FrameObjectRec* Frame, long Index)
	{
		NoteObjectRec**			NewArray;

		CheckPtrExistence(Frame);
		NewArray = (NoteObjectRec**)RemoveEntryFromArrayCopy((char*)(Frame->NoteArray),
			Index,sizeof(NoteObjectRec*));
		if (NewArray != NIL)
			{
				ReleasePtr((char*)(Frame->NoteArray));
				Frame->NoteArray = NewArray;
				Frame->Width = 0;
				return True;
			}
		 else
			{
				return False;
			}
	}


/* append a note to the frame.  returns True if successful. */
MyBoolean							AppendNoteToFrame(FrameObjectRec* Frame, struct NoteObjectRec* Note)
	{
		NoteObjectRec**			NewRef;
		long								OldSize;

		CheckPtrExistence(Frame);
		CheckPtrExistence(Note);
		OldSize = PtrSize((char*)(Frame->NoteArray));
		ERROR(IsThisACommandFrame(Frame),PRERR(ForceAbort,
			"AppendNoteToFrame:  adding note to command frame"));
		ERROR(!IsThisACommandFrame(Frame) && (NumNotesInFrame(Frame) > 0)
			&& IsItACommand(Note),PRERR(ForceAbort,
			"AppendNoteToFrame:  adding command to note frame"));
		NewRef = (NoteObjectRec**)ResizePtr((char*)(Frame->NoteArray),
			OldSize + sizeof(NoteObjectRec*));
		if (NewRef == NIL)
			{
				return False;
			}
		Frame->NoteArray = NewRef;
		Frame->NoteArray[OldSize / sizeof(NoteObjectRec*)] = Note;
		Frame->Width = 0;
		return True;
	}


/* find out if this is a command frame.  command frames are allowed to have a */
/* single command and nothing else in them */
MyBoolean							IsThisACommandFrame(FrameObjectRec* Frame)
	{
		CheckPtrExistence(Frame);
		return (NumNotesInFrame(Frame) == 1) && IsItACommand(Frame->NoteArray[0]);
	}


/* find out the width of this command/note frame and draw it if the flag is set. */
/* it assumes the clipping rectangle is set up properly.  the X and Y parameters */
/* specify the left edge of the note and the Middle C line. */
/* this routine does not handle drawing of ties. */
OrdType								WidthOfFrameAndDraw(WinType* Window, OrdType X, OrdType Y,
												FontType Font, FontSizeType FontSize, OrdType FontHeight,
												FrameObjectRec* Frame, MyBoolean ActuallyDraw,
												MyBoolean GreyedOut)
	{
		OrdType							Width;

		/* this caching of the width of the frame should speed things up */
		if ((Frame->Width != 0) && !ActuallyDraw)
			{
				/* we can only do this if we aren't drawing! */
				return Frame->Width;
			}
		/* we should be able to find ways of overlapping the notes if they won't */
		/* be on top of each other on screen but we're not going to for now. */
		if (IsThisACommandFrame(Frame))
			{
				/* it's a command frame, so draw using the special command drawing routines */
				Width = DrawCommandOnScreen(Window,X,Y,Font,FontSize,FontHeight,
					GetNoteFromFrame(Frame,0),ActuallyDraw,GreyedOut);
			}
		 else
			{
				long								Limit;
				long								Scan;

				/* we have to draw the notes ourselves. */
				X -= LEFTNOTEEDGEINSET;
				Width = 0;
				Limit = NumNotesInFrame(Frame);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						NoteObjectRec*			Note;

						/* get the note to be drawn */
						Note = GetNoteFromFrame(Frame,Scan);
						CheckPtrExistence(Note);
						/* do the icon stuff */
						if (ActuallyDraw)
							{
								Bitmap*							Image;
								Bitmap*							Mask;
								OrdType							NoteOffset;

								/* first, obtain the proper image for the duration */
								if (!GetNoteIsItARest(Note))
									{
										switch (GetNoteDuration(Note))
											{
												default:
													EXECUTE(PRERR(ForceAbort,
														"LengthOfCommandFrameAndDraw:  bad duration value"));
													break;
												case e64thNote:
													Image = SixtyFourthNoteImage;
													Mask = SixtyFourthNoteMask;
													break;
												case e32ndNote:
													Image = ThirtySecondNoteImage;
													Mask = ThirtySecondNoteMask;
													break;
												case e16thNote:
													Image = SixteenthNoteImage;
													Mask = SixteenthNoteMask;
													break;
												case e8thNote:
													Image = EighthNoteImage;
													Mask = EighthNoteMask;
													break;
												case e4thNote:
													Image = QuarterNoteImage;
													Mask = QuarterNoteMask;
													break;
												case e2ndNote:
													Image = HalfNoteImage;
													Mask = HalfNoteMask;
													break;
												case eWholeNote:
													Image = WholeNoteImage;
													Mask = WholeNoteMask;
													break;
												case eDoubleNote:
													Image = DoubleNoteImage;
													Mask = DoubleNoteMask;
													break;
												case eQuadNote:
													Image = QuadNoteImage;
													Mask = QuadNoteMask;
													break;
											}
									}
								 else
									{
										switch (GetNoteDuration(Note))
											{
												default:
													EXECUTE(PRERR(ForceAbort,
														"LengthOfCommandFrameAndDraw:  bad duration value"));
													break;
												case e64thNote:
													Image = SixtyFourthRestImage;
													Mask = SixtyFourthRestMask;
													break;
												case e32ndNote:
													Image = ThirtySecondRestImage;
													Mask = ThirtySecondRestMask;
													break;
												case e16thNote:
													Image = SixteenthRestImage;
													Mask = SixteenthRestMask;
													break;
												case e8thNote:
													Image = EighthRestImage;
													Mask = EighthRestMask;
													break;
												case e4thNote:
													Image = QuarterRestImage;
													Mask = QuarterRestMask;
													break;
												case e2ndNote:
													Image = HalfRestImage;
													Mask = HalfRestMask;
													break;
												case eWholeNote:
													Image = WholeRestImage;
													Mask = WholeRestMask;
													break;
												case eDoubleNote:
													Image = DoubleRestImage;
													Mask = DoubleRestMask;
													break;
												case eQuadNote:
													Image = QuadRestImage;
													Mask = QuadRestMask;
													break;
											}
									}
								/* duplicate images so we can modify them */
								Image = DuplicateBitmap(Image);
								if (Image == NIL)
									{
										goto FailurePoint;
									}
								Mask = DuplicateBitmap(Mask);
								if (Mask == NIL)
									{
										DisposeBitmap(Image);
										goto FailurePoint;
									}
								/* now, handle divisions */
								switch (GetNoteDurationDivision(Note))
									{
										default:
											EXECUTE(PRERR(ForceAbort,
												"LengthOfCommandFrameAndDraw:  bad division value"));
											break;
										case eDiv1Modifier:
											/* no change */
											break;
										case eDiv3Modifier:
											BitmapOrIntoBitmap(Div3Image,Image);
											BitmapOrIntoBitmap(Div3Mask,Mask);
											break;
										case eDiv5Modifier:
											BitmapOrIntoBitmap(Div5Image,Image);
											BitmapOrIntoBitmap(Div5Mask,Mask);
											break;
										case eDiv7Modifier:
											BitmapOrIntoBitmap(Div7Image,Image);
											BitmapOrIntoBitmap(Div7Mask,Mask);
											break;
									}
								/* handle dots */
								if (GetNoteDotStatus(Note))
									{
										BitmapOrIntoBitmap(DotImage,Image);
										BitmapOrIntoBitmap(DotMask,Mask);
									}
								/* sharps and flats require more clever handling */
								if ((GetNoteFlatOrSharpStatus(Note) & eSharpModifier) != 0)
									{
										BitmapOrIntoBitmap(SharpImage,Image);
										BitmapOrIntoBitmap(SharpMask,Mask);
									}
								if ((GetNoteFlatOrSharpStatus(Note) & eFlatModifier) != 0)
									{
										BitmapOrIntoBitmap(FlatImage,Image);
										BitmapOrIntoBitmap(FlatMask,Mask);
									}
								/* check greying status */
								if (GreyedOut)
									{
										BitmapAndIntoBitmap(GreyMask,Image);
									}
								/* perform the drawing */
								NoteOffset = Y + ConvertPitchToPixel(GetNotePitch(Note),
									GetNoteFlatOrSharpStatus(Note))
									- TOPNOTESTAFFINTERSECT - GetCenterNotePixel();
								BicBitmap(Window,X,NoteOffset,Mask);
								OrBitmap(Window,X,NoteOffset,Image);
								/* dump the bitmaps */
								DisposeBitmap(Mask);
								DisposeBitmap(Image);
								/* increment X for the next time around */
								X += INTERNALSEPARATION;
							}

						/* update the width count */
					 FailurePoint:
						if (Width == 0)
							{
								/* first time you get the whole width */
								Width = ICONWIDTH - LEFTNOTEEDGEINSET;
							}
						 else
							{
								/* other times, you just get whatever extra there is */
								Width += INTERNALSEPARATION;
							}
					}
			}
		Frame->Width = Width;
		return Width;
	}


/* find out the duration of the specified frame.  returns the duration of the */
/* frame as a fraction */
void									DurationOfFrame(FrameObjectRec* Frame, struct FractionRec* Frac)
	{
		long								Scan;
		long								Limit;
		NoteObjectRec*			Note;

		CheckPtrExistence(Frame);
		if (IsThisACommandFrame(Frame))
			{
				Frac->Integer = 0;
				Frac->Fraction = 0;
				Frac->Denominator = (64*3*5*7*2);
				return;
			}
		Limit = NumNotesInFrame(Frame);
		ERROR(Limit < 1,PRERR(ForceAbort,"DurationOfFrame called on empty frame"));
		/* obtain duration of first element */
		Note = GetNoteFromFrame(Frame,0);
		GetNoteDurationFrac(Note,Frac);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				FractionRec					TempFrac;

				Note = GetNoteFromFrame(Frame,Scan);
				GetNoteDurationFrac(Note,&TempFrac);
				if (FracGreaterThan(Frac,&TempFrac))
					{
						/* choose smallest one */
						*Frac = TempFrac;
					}
			}
	}


/* make a total (deep) copy of the frame and the notes it contains */
FrameObjectRec*				DeepDuplicateFrame(FrameObjectRec* Frame)
	{
		FrameObjectRec*			Copy;
		long								Limit;
		long								Scan;

		CheckPtrExistence(Frame);
		Copy = NewFrame();
		if (Copy == NIL)
			{
			 FailurePoint1:
				return NIL;
			}

		Limit = NumNotesInFrame(Frame);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				NoteObjectRec*			NoteCopy;

				NoteCopy = DeepCopyNoteObject(GetNoteFromFrame(Frame,Scan));
				if (NoteCopy == NIL)
					{
					 FailurePoint2:
						DisposeFrameAndContents(Copy);
						goto FailurePoint1;
					}
				if (!AppendNoteToFrame(Copy,NoteCopy))
					{
					 FailurePoint2a:
						DisposeNote(NoteCopy);
						goto FailurePoint2;
					}
			}

		return Copy;
	}
