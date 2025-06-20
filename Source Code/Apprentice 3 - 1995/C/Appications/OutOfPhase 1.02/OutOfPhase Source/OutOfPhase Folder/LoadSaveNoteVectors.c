/* LoadSaveNoteVectors.c */
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

#include "LoadSaveNoteVectors.h"
#include "Array.h"
#include "FrameObject.h"
#include "NoteObject.h"
#include "Memory.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"


/* Note Vector Subblock Format: */
/*   1-byte format version number */
/*       should be 1 */
/*   4-byte little endian number of frames in the vector */
/*   * for each frame: */
/*       4-byte little endian number of notes in the frame */
/*       n-bytes of data for all of the notes (see note object format) */
/*   4-byte little endian number of records in the tie matrix */
/*   * for each tie matrix entry: */
/*       4-byte little endian index of the source frame */
/*       4-byte little endian index of the source note in the frame */
/*       4-byte little endian index of the target frame */
/*       4-byte little endian index of the target note in the frame */


/* this reads in notes from the file and the tie matrix and builts a note */
/* vector from the information. */
FileLoadingErrors		ReadNoteVector(struct ArrayRec** FrameArrayOut,
											struct BufferedInputRec* Input)
	{
		signed long				NumberOfFrames;
		long							FrameScan;
		ArrayRec*					FrameArray;
		FileLoadingErrors	Error;
		signed long				NumberOfTieRecords;
		long							TieScan;
		unsigned char			UnsignedChar;

		CheckPtrExistence(Input);

		/*   1-byte format version number */
		/*       should be 1 */
		if (!ReadBufferedUnsignedChar(Input,&UnsignedChar))
			{
				Error = eFileLoadDiskError;
			 FailurePointneg1:
				return Error;
			}
		if (UnsignedChar != 1)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint0:
				goto FailurePointneg1;
			}

		FrameArray = NewArray();
		if (FrameArray == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint1:
				goto FailurePoint0;
			}

		/*   4-byte little endian number of frames in the vector */
		if (!ReadBufferedSignedLongLittleEndian(Input,&NumberOfFrames))
			{
				long							FrameLimit;

				Error = eFileLoadDiskError;
			 FailurePoint2:
				FrameScan = ArrayGetLength(FrameArray);
				for (FrameScan = 0; FrameScan < FrameLimit; FrameScan += 1)
					{
						DisposeFrameAndContents((FrameObjectRec*)ArrayGetElement(
							FrameArray,FrameScan));
					}
				DisposeArray(FrameArray);
				goto FailurePoint1;
			}
		if (NumberOfFrames < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint3:
				goto FailurePoint2;
			}

		/*   * for each frame: */
		/*       4-byte little endian number of notes in the frame */
		/*       n-bytes of data for all of the notes */
		for (FrameScan = 0; FrameScan < NumberOfFrames; FrameScan += 1)
			{
				signed long					NumberOfNotes;
				long								NoteScan;
				FrameObjectRec*			FrameObject;

				if (!ReadBufferedSignedLongLittleEndian(Input,&NumberOfNotes))
					{
						Error = eFileLoadDiskError;
					 FailurePoint4:
						goto FailurePoint3;
					}

				FrameObject = NewFrame();
				if (FrameObject == NIL)
					{
						Error = eFileLoadOutOfMemory;
					 FailurePoint4a:
						goto FailurePoint4;
					}

				for (NoteScan = 0; NoteScan < NumberOfNotes; NoteScan += 1)
					{
						NoteObjectRec*			Note;

						Error = NoteObjectNewFromFile(&Note,Input);
						if (Error != eFileLoadNoError)
							{
							 FailurePoint4b:
								DisposeFrameAndContents(FrameObject);
								goto FailurePoint4a;
							}

						if (((NumNotesInFrame(FrameObject) > 1) && IsItACommand(Note))
							|| IsThisACommandFrame(FrameObject))
							{
								Error = eFileLoadBadFormat;
							 FailurePoint4ba:
								DisposeNote(Note);
								goto FailurePoint4b;
							}

						if (!AppendNoteToFrame(FrameObject,Note))
							{
								Error = eFileLoadOutOfMemory;
							 FailurePoint4bb:
								goto FailurePoint4ba;
							}
					}

				if (!ArrayAppendElement(FrameArray,FrameObject))
					{
						Error = eFileLoadOutOfMemory;
						goto FailurePoint4b;
					}
			}

		/*   4-byte little endian number of records in the tie matrix */
		if (!ReadBufferedSignedLongLittleEndian(Input,&NumberOfTieRecords))
			{
				Error = eFileLoadDiskError;
			 FailurePoint5:
				goto FailurePoint4;
			}
		if (NumberOfTieRecords < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint6:
				goto FailurePoint5;
			}

		/*   * for each tie matrix entry: */
		/*       4-byte little endian index of the source frame */
		/*       4-byte little endian index of the source note in the frame */
		/*       4-byte little endian index of the target frame */
		/*       4-byte little endian index of the target note in the frame */
		for (TieScan = 0; TieScan < NumberOfTieRecords; TieScan += 1)
			{
				signed long					SourceFrameIndex;
				signed long					SourceNoteIndex;
				signed long					TargetFrameIndex;
				signed long					TargetNoteIndex;
				FrameObjectRec*			SourceFrame;
				FrameObjectRec*			TargetFrame;
				NoteObjectRec*			SourceNote;
				NoteObjectRec*			TargetNote;

				if (!ReadBufferedSignedLongLittleEndian(Input,&SourceFrameIndex))
					{
						Error = eFileLoadDiskError;
					 FailurePoint7:
						goto FailurePoint6;
					}
				if (SourceFrameIndex < 0)
					{
						Error = eFileLoadBadFormat;
					 FailurePoint7a:
						goto FailurePoint7;
					}
				if (!ReadBufferedSignedLongLittleEndian(Input,&SourceNoteIndex))
					{
						Error = eFileLoadDiskError;
					 FailurePoint7b:
						goto FailurePoint7a;
					}
				if (SourceNoteIndex < 0)
					{
						Error = eFileLoadBadFormat;
					 FailurePoint7c:
						goto FailurePoint7b;
					}
				if (!ReadBufferedSignedLongLittleEndian(Input,&TargetFrameIndex))
					{
						Error = eFileLoadDiskError;
					 FailurePoint7d:
						goto FailurePoint7c;
					}
				if (TargetFrameIndex < 0)
					{
						Error = eFileLoadBadFormat;
					 FailurePoint7e:
						goto FailurePoint7d;
					}
				if (!ReadBufferedSignedLongLittleEndian(Input,&TargetNoteIndex))
					{
						Error = eFileLoadDiskError;
					 FailurePoint7f:
						goto FailurePoint7e;
					}
				if (TargetNoteIndex < 0)
					{
						Error = eFileLoadBadFormat;
					 FailurePoint7g:
						goto FailurePoint7f;
					}
				if ((TargetFrameIndex <= SourceFrameIndex)
					|| (TargetFrameIndex >= ArrayGetLength(FrameArray))
					|| (SourceFrameIndex >= ArrayGetLength(FrameArray)))
					{
						Error = eFileLoadBadFormat;
					 FailurePoint7h:
						goto FailurePoint7g;
					}
				SourceFrame = (FrameObjectRec*)ArrayGetElement(FrameArray,SourceFrameIndex);
				CheckPtrExistence(SourceFrame);
				TargetFrame = (FrameObjectRec*)ArrayGetElement(FrameArray,TargetFrameIndex);
				CheckPtrExistence(TargetFrame);
				if ((SourceNoteIndex >= NumNotesInFrame(SourceFrame))
					|| (TargetNoteIndex >= NumNotesInFrame(TargetFrame)))
					{
						Error = eFileLoadBadFormat;
					 FailurePoint7i:
						goto FailurePoint7h;
					}
				SourceNote = GetNoteFromFrame(SourceFrame,SourceNoteIndex);
				CheckPtrExistence(SourceNote);
				TargetNote = GetNoteFromFrame(TargetFrame,TargetNoteIndex);
				CheckPtrExistence(TargetNote);
				if (IsItACommand(SourceNote) || IsItACommand(TargetNote))
					{
						Error = eFileLoadBadFormat;
					 FailurePoint7j:
						goto FailurePoint7i;
					}
				PutNoteTieTarget(SourceNote,TargetNote);
			}

		*FrameArrayOut = FrameArray;
		return eFileLoadNoError;
	}


/* this writes out the information for each note and then writes the tie matrix */
/* to the file. */
FileLoadingErrors		WriteNoteVector(struct ArrayRec* ArrayOfFrames,
											struct BufferedOutputRec* Output)
	{
		long							NumberOfFrames;
		long							FrameScan;
		long							NumberOfTieRecords;

		CheckPtrExistence(ArrayOfFrames);
		CheckPtrExistence(Output);

		/*   1-byte format version number */
		/*       should be 1 */
		if (!WriteBufferedUnsignedChar(Output,1))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian number of frames in the vector */
		NumberOfFrames = ArrayGetLength(ArrayOfFrames);
		if (!WriteBufferedSignedLongLittleEndian(Output,NumberOfFrames))
			{
				return eFileLoadDiskError;
			}

		/*   * for each frame: */
		/*       4-byte little endian number of notes in the frame */
		/*       n-bytes of data for all of the notes */
		NumberOfTieRecords = 0;
		for (FrameScan = 0; FrameScan < NumberOfFrames; FrameScan += 1)
			{
				FrameObjectRec*		Frame;
				long							NumberOfNotes;
				long							NoteScan;
				FileLoadingErrors	Error;

				Frame = (FrameObjectRec*)ArrayGetElement(ArrayOfFrames,FrameScan);
				CheckPtrExistence(Frame);
				NumberOfNotes = NumNotesInFrame(Frame);
				if (!WriteBufferedSignedLongLittleEndian(Output,NumberOfNotes))
					{
						return eFileLoadDiskError;
					}
				for (NoteScan = 0; NoteScan < NumberOfNotes; NoteScan += 1)
					{
						NoteObjectRec*		Note;

						Note = GetNoteFromFrame(Frame,NoteScan);
						if (!IsItACommand(Note) && (GetNoteTieTarget(Note) != NIL))
							{
								NumberOfTieRecords += 1;
							}
						Error = NoteObjectWriteDataOut(Note,Output);
						if (Error != eFileLoadNoError)
							{
								return Error;
							}
					}
			}

		/*   4-byte little endian number of records in the tie matrix */
		if (!WriteBufferedSignedLongLittleEndian(Output,NumberOfTieRecords))
			{
				return eFileLoadDiskError;
			}

		/*   * for each tie matrix entry: */
		/*       4-byte little endian index of the source frame */
		/*       4-byte little endian index of the source note in the frame */
		/*       4-byte little endian index of the target frame */
		/*       4-byte little endian index of the target note in the frame */
		for (FrameScan = 0; FrameScan < NumberOfFrames; FrameScan += 1)
			{
				FrameObjectRec*		Frame;
				long							NumberOfNotes;
				long							NoteScan;

				Frame = (FrameObjectRec*)ArrayGetElement(ArrayOfFrames,FrameScan);
				CheckPtrExistence(Frame);
				NumberOfNotes = NumNotesInFrame(Frame);
				for (NoteScan = 0; NoteScan < NumberOfNotes; NoteScan += 1)
					{
						NoteObjectRec*		Note;

						Note = GetNoteFromFrame(Frame,NoteScan);
						if (!IsItACommand(Note))
							{
								NoteObjectRec*		TieTarget;

								TieTarget = GetNoteTieTarget(Note);
								if (TieTarget != NIL)
									{
										long							SubFrameScan;

										NumberOfTieRecords -= 1;
										for (SubFrameScan = FrameScan + 1; SubFrameScan < NumberOfFrames;
											SubFrameScan += 1)
											{
												FrameObjectRec*		SearchFrame;
												long							SubNumNotes;
												long							SubNoteScan;

												SearchFrame = (FrameObjectRec*)ArrayGetElement(ArrayOfFrames,
													SubFrameScan);
												CheckPtrExistence(SearchFrame);
												SubNumNotes = NumNotesInFrame(SearchFrame);
												for (SubNoteScan = 0; SubNoteScan < SubNumNotes; SubNoteScan += 1)
													{
														NoteObjectRec*		SearchNote;

														SearchNote = GetNoteFromFrame(SearchFrame,SubNoteScan);
														if (!IsItACommand(SearchNote))
															{
																if (SearchNote == TieTarget)
																	{
																		if (!WriteBufferedSignedLongLittleEndian(
																			Output,FrameScan))
																			{
																				return eFileLoadDiskError;
																			}
																		if (!WriteBufferedSignedLongLittleEndian(
																			Output,NoteScan))
																			{
																				return eFileLoadDiskError;
																			}
																		if (!WriteBufferedSignedLongLittleEndian(
																			Output,SubFrameScan))
																			{
																				return eFileLoadDiskError;
																			}
																		if (!WriteBufferedSignedLongLittleEndian(
																			Output,SubNoteScan))
																			{
																				return eFileLoadDiskError;
																			}
																		goto DoneSearchingForTieTargetPoint;
																	}
															}
													}
											}
										EXECUTE(PRERR(ForceAbort,
											"WriteNoteVector:  tie target couldn't be found"));
										/* jump out here when tie target has been found */
									 DoneSearchingForTieTargetPoint:
										;
									}
							}
					}
			}
		ERROR(NumberOfTieRecords != 0,PRERR(ForceAbort,
			"WriteNoteVector:  tie record count inconsistency"));

		return eFileLoadNoError;
	}
