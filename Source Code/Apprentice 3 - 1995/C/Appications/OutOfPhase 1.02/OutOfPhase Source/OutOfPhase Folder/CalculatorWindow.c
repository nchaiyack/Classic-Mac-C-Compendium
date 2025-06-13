/* CalculatorWindow.c */
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

#include "CalculatorWindow.h"
#include "Screen.h"
#include "TextEdit.h"
#include "Memory.h"
#include "MainWindowStuff.h"
#include "GrowIcon.h"
#include "Main.h"
#include "SimpleButton.h"
#include "Alert.h"
#include "CodeCenter.h"
#include "PcodeStack.h"
#include "PcodeSystem.h"
#include "DataMunging.h"
#include "Numbers.h"
#include "FixedPoint.h"
#include "WindowDispatcher.h"
#include "GlobalWindowMenuList.h"
#include "FunctionCode.h"
#include "CompilerRoot.h"
#include "PcodeDisassembly.h"
#include "DisassemblyWindow.h"


#define WINDOWLEFT (30)
#define WINDOWTOP (30)
#define WINDOWWIDTH (450)
#define WINDOWHEIGHT (200)

#define BUTTONLEFT (10)
#define BUTTONTOP (2)
#define BUTTONWIDTH (120)
#define BUTTONHEIGHT (19)

struct CalcWindowRec
	{
		MainWindowRec*			MainWindow;
		CodeCenterRec*			CodeCenter;

		WinType*						ScreenID;
		GenericWindowRec*		MyGenericWindow; /* how the window event dispatcher knows us */
		MenuItemType*				MyMenuItem;
		TextEditRec*				Editor;
		long								LastLine;
		SimpleButtonRec*		EvalButton;
	};


/* create a new calculator window.  the caller is responsible for registering the */
/* new calculator with the main window. */
CalcWindowRec*			NewCalculatorWindow(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter)
	{
		CalcWindowRec*		Window;

		CheckPtrExistence(MainWindow);
		CheckPtrExistence(CodeCenter);
		Window = (CalcWindowRec*)AllocPtrCanFail(sizeof(CalcWindowRec),"CalcWindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Window->MainWindow = MainWindow;
		Window->CodeCenter = CodeCenter;
		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,
			eWindowZoomable,eWindowResizable,WINDOWLEFT,WINDOWTOP,WINDOWWIDTH,WINDOWHEIGHT,
			(void (*)(void*))&CalculatorWindowUpdator,Window);
		if (Window->ScreenID == 0)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		SetWindowName(Window->ScreenID,"Calculator");
		Window->Editor = NewTextEdit(Window->ScreenID,
			(TEScrollType)(eTEVScrollBar | eTEHScrollBar),GetMonospacedFont(),9,
			-1,(2 * BUTTONTOP) + BUTTONHEIGHT,WINDOWWIDTH + 2,
			WINDOWHEIGHT + 1 - ((2 * BUTTONTOP) + BUTTONHEIGHT));
		if (Window->Editor == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		Window->LastLine = 0;
		Window->EvalButton = NewSimpleButton(Window->ScreenID,"Evaluate",
			BUTTONLEFT,BUTTONTOP,BUTTONWIDTH,BUTTONHEIGHT);
		if (Window->EvalButton == NIL)
			{
			 FailurePoint4:
				DisposeTextEdit(Window->Editor);
				goto FailurePoint3;
			}
		Window->MyGenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&CalculatorWindowDoIdle,
			(void (*)(void*))&CalculatorWindowBecomeActive,
			(void (*)(void*))&CalculatorWindowBecomeInactive,
			(void (*)(void*))&CalculatorWindowJustResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&CalculatorWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&CalculatorWindowDoKeyDown,
			(void (*)(void*))&CalculatorWindowClose,
			(void (*)(void*))&CalculatorWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&CalculatorWindowDoMenuCommand);
		if (Window->MyGenericWindow == NIL)
			{
			 FailurePoint5:
				DisposeSimpleButton(Window->EvalButton);
				goto FailurePoint4;
			}
		Window->MyMenuItem = MakeNewMenuItem(mmWindowMenu,"Calculator",0);
		if (Window->MyMenuItem == NIL)
			{
			 FailurePoint6:
				CheckOutDyingWindow(Window->MyGenericWindow);
				goto FailurePoint5;
			}
		if (!RegisterWindowMenuItem(Window->MyMenuItem,(void (*)(void*))&ActivateThisWindow,
			Window->ScreenID))
			{
			 FailurePoint7:
				KillMenuItem(Window->MyMenuItem);
				goto FailurePoint6;
			}
		SetTextEditAutoIndent(Window->Editor,True);
		SetTextEditTabSize(Window->Editor,MainWindowGetTabSize(MainWindow));
		return Window;
	}


/* dispose of a calculator window.  the calculator notifies the main window that */
/* owns it. */
void								DisposeCalculatorWindow(CalcWindowRec* Window)
	{
		CheckPtrExistence(Window);
		DeregisterWindowMenuItem(Window->MyMenuItem);
		KillMenuItem(Window->MyMenuItem);
		CheckOutDyingWindow(Window->MyGenericWindow);
		DisposeSimpleButton(Window->EvalButton);
		DisposeTextEdit(Window->Editor);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}


void								CalculatorWindowDoIdle(CalcWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		TextEditUpdateCursor(Window->Editor);
		if (CheckCursorFlag)
			{
				if (TextEditIBeamTest(Window->Editor,XLoc,YLoc))
					{
						SetIBeamCursor();
					}
				 else
					{
						SetArrowCursor();
					}
			}
	}


void								CalculatorWindowBecomeActive(CalcWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		EnableTextEditSelection(Window->Editor);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,GetGrowIcon(True/*enablegrowicon*/));
	}


void								CalculatorWindowBecomeInactive(CalcWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		DisableTextEditSelection(Window->Editor);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,GetGrowIcon(False/*disablegrowicon*/));
	}


void								CalculatorWindowJustResized(CalcWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,0,0,XSize,YSize);
		DrawBoxErase(Window->ScreenID,0,0,XSize,YSize);
		SetTextEditPosition(Window->Editor,-1,(2 * BUTTONTOP) + BUTTONHEIGHT,
			XSize + 2,YSize + 1 - ((2 * BUTTONTOP) + BUTTONHEIGHT));
	}


void								CalculatorWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, CalcWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((XLoc >= GetWindowWidth(Window->ScreenID) - 15)
			&& (XLoc < GetWindowWidth(Window->ScreenID))
			&& (YLoc >= GetWindowHeight(Window->ScreenID) - 15)
			&& (YLoc < GetWindowHeight(Window->ScreenID)))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				CalculatorWindowJustResized(Window);
			}
		else if (TextEditHitTest(Window->Editor,XLoc,YLoc))
			{
				TextEditDoMouseDown(Window->Editor,XLoc,YLoc,Modifiers);
			}
		else if (SimpleButtonHitTest(Window->EvalButton,XLoc,YLoc))
			{
				if (SimpleButtonMouseDown(Window->EvalButton,XLoc,YLoc,NIL,NIL))
					{
						CalculatorWindowDoCalculation(Window);
					}
			}
	}


void								CalculatorWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, CalcWindowRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditDoKeyPressed(Window->Editor,KeyCode,Modifiers);
	}


void								CalculatorWindowClose(CalcWindowRec* Window)
	{
		CheckPtrExistence(Window);
		/* notification of closing is here and not in dispose because FunctionWindow */
		/* calls dispose, so it knows that we are dying, but this routine handles */
		/* a user close, which FunctionWindow doesn't know about */
		MainWindowCalculatorClosingNotify(Window->MainWindow,Window);
		DisposeCalculatorWindow(Window);
	}


void								CalculatorWindowUpdator(CalcWindowRec* Window)
	{
		OrdType			XSize;
		OrdType			YSize;

		CheckPtrExistence(Window);
		TextEditFullRedraw(Window->Editor);
		RedrawSimpleButton(Window->EvalButton);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,
			GetGrowIcon(Window->MyGenericWindow == GetCurrentWindowID()));
	}


void								CalculatorWindowMenuSetup(CalcWindowRec* Window)
	{
		CheckPtrExistence(Window);
		MainWindowEnableGlobalMenus(Window->MainWindow);
		EnableMenuItem(mPaste);
		ChangeItemName(mPaste,"Paste Text");
		if (TextEditIsThereValidSelection(Window->Editor))
			{
				EnableMenuItem(mCut);
				ChangeItemName(mCut,"Cut Text");
				EnableMenuItem(mCopy);
				ChangeItemName(mCopy,"Copy Text");
				EnableMenuItem(mClear);
				ChangeItemName(mClear,"Clear Text");
			}
		EnableMenuItem(mShiftLeft);
		EnableMenuItem(mShiftRight);
		EnableMenuItem(mBalanceParens);
		EnableMenuItem(mSelectAll);
		ChangeItemName(mSelectAll,"Select All Text");
		if (TextEditCanWeUndo(Window->Editor))
			{
				EnableMenuItem(mUndo);
				ChangeItemName(mUndo,"Undo Text Change");
			}
		ChangeItemName(mCloseFile,"Close Calculator");
		EnableMenuItem(mCloseFile);
		EnableMenuItem(mEvaluateCalc);
		SetItemCheckmark(Window->MyMenuItem);
		EnableMenuItem(mDisassembleFunction);
	}


void								CalculatorWindowDoMenuCommand(CalcWindowRec* Window,
											MenuItemType* MenuItem)
	{
		CheckPtrExistence(Window);
		if (MainWindowDoGlobalMenuItem(Window->MainWindow,MenuItem))
			{
			}
		else if (MenuItem == mPaste)
			{
				TextEditDoMenuPaste(Window->Editor);
			}
		else if (MenuItem == mCut)
			{
				TextEditDoMenuCut(Window->Editor);
			}
		else if (MenuItem == mCopy)
			{
				TextEditDoMenuCopy(Window->Editor);
			}
		else if (MenuItem == mClear)
			{
				TextEditDoMenuClear(Window->Editor);
			}
		else if (MenuItem == mSelectAll)
			{
				TextEditDoMenuSelectAll(Window->Editor);
			}
		else if (MenuItem == mUndo)
			{
				TextEditDoMenuUndo(Window->Editor);
			}
		else if (MenuItem == mCloseFile)
			{
				CalculatorWindowClose(Window);
			}
		else if (MenuItem == mShiftLeft)
			{
				TextEditShiftSelectionLeftOneTab(Window->Editor);
			}
		else if (MenuItem == mShiftRight)
			{
				TextEditShiftSelectionRightOneTab(Window->Editor);
			}
		else if (MenuItem == mBalanceParens)
			{
				TextEditBalanceParens(Window->Editor);
			}
		else if (MenuItem == mEvaluateCalc)
			{
				CalculatorWindowDoCalculation(Window);
			}
		else if (MenuItem == mDisassembleFunction)
			{
				char*								Blob;
				PcodeRec*						FuncCode;
				CompileErrors				Error;
				long								LineNumber;
				DataTypes						ReturnType;
				char*								DisassemblyText;

				/* prepare the text blob to be evaluated */
				if (!TextEditIsThereValidSelection(Window->Editor))
					{
						if (Window->LastLine > GetTextEditNumLines(Window->Editor))
							{
								Window->LastLine = GetTextEditNumLines(Window->Editor);
							}
						SetTextEditSelection(Window->Editor,Window->LastLine,0,
							GetTextEditNumLines(Window->Editor),0);
					}
				Blob = TextEditGetSelection(Window->Editor);
				if (Blob == NIL)
					{
					 FailurePoint1:
						AlertHalt("There is not enough memory available to compile "
							"the expression.",NIL);
						return;
					}

				/* perform compilation */
				EXECUTE(FuncCode = NIL;)
				Error = CompileSpecialFunction(NIL/*no arguments*/,0/*noargs*/,&LineNumber,
					&ReturnType,Blob/*text*/,&FuncCode);
				ReleasePtr(Blob);
				if (Error != eCompileNoError)
					{
						ERROR(FuncCode != NIL,PRERR(ForceAbort,"CalculatorWindowDoMenuCommand:  "
							"compile failed, but function is not NIL."));
						SetTextEditSelection(Window->Editor,
							GetTextEditSelectStartLine(Window->Editor) + LineNumber,0,
							GetTextEditSelectStartLine(Window->Editor) + LineNumber + 1,0);
						TextEditShowSelection(Window->Editor);
						AlertHalt("A compile error occurred:  _",GetCompileErrorString(Error));
						return;
					}

				/* obtain the disassembly */
				DisassemblyText = DisassemblePcode(FuncCode,'\x0a');
				DisposePcode(FuncCode);
				if (DisassemblyText == NIL)
					{
						AlertHalt("There is not enough memory available to disassemble "
							"the expression.",NIL);
						return;
					}

				/* show it */
				if (NewDisassemblyWindow(DisassemblyText,Window->MainWindow) == NIL)
					{
						AlertHalt("There is not enough memory available to "
							"show the disassembly window.",NIL);
					}
				ReleasePtr(DisassemblyText);
			}
		else
			{
				EXECUTE(PRERR(AllowResume,"CalculatorWindowDoMenuCommand:  unknown menu command"));
			}
	}


/* perform evaluation.  if there is a selection, then evaluate */
/* the selection, otherwise evaluate from last point */
void								CalculatorWindowDoCalculation(CalcWindowRec* Window)
	{
		char*								Blob;
		PcodeRec*						FuncCode;
		CompileErrors				Error;
		long								LineNumber;
		DataTypes						ReturnType;
		ParamStackRec*			ParamList;
		EvalErrors					OtherError;
		OpcodeRec*					ErrorOpcode;
		long								OffendingInstruction;
		char*								Number;
		char*								Statement;
		MyBoolean						MemoryErrorOccurred;

		CheckPtrExistence(Window);

		/* bring the world up to date */
		if (!MainWindowMakeUpToDateFunctions(Window->MainWindow))
			{
				return;
			}

		/* prepare the text blob to be evaluated */
		if (!TextEditIsThereValidSelection(Window->Editor))
			{
				if (Window->LastLine > GetTextEditNumLines(Window->Editor))
					{
						Window->LastLine = GetTextEditNumLines(Window->Editor);
					}
				SetTextEditSelection(Window->Editor,Window->LastLine,0,
					GetTextEditNumLines(Window->Editor),0);
			}
		Blob = TextEditGetSelection(Window->Editor);
		if (Blob == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to compile the expression.",NIL);
				return;
			}

		/* perform compilation */
		EXECUTE(FuncCode = NIL;)
		Error = CompileSpecialFunction(NIL/*no arguments*/,0/*noargs*/,&LineNumber,
			&ReturnType,Blob/*text*/,&FuncCode);
		ReleasePtr(Blob);
		if (Error != eCompileNoError)
			{
				ERROR(FuncCode != NIL,PRERR(ForceAbort,
					"CalculatorWindowDoCalculation:  compile failed, but function is not NIL."));
				SetTextEditSelection(Window->Editor,GetTextEditSelectStartLine(Window->Editor)
					+ LineNumber,0,GetTextEditSelectStartLine(Window->Editor) + LineNumber + 1,0);
				TextEditShowSelection(Window->Editor);
				AlertHalt("A compile error occurred:  _",GetCompileErrorString(Error));
				return;
			}

		/* try to evaluate the code */
		ParamList = NewParamStack();
		if (ParamList == NIL)
			{
			 SecondFailurePoint1:
				DisposePcode(FuncCode);
				AlertHalt("There is not enough memory available to evaluate the expression.",NIL);
				return;
			}
		/* add a space for the return value */
		if (!AddIntegerToStack(ParamList,0))
			{
			 SecondFailurePoint2:
				goto SecondFailurePoint1;
			}
		/* executing the actual code */
		OtherError = EvaluatePcode(ParamList,FuncCode,
			Window->CodeCenter,&ErrorOpcode,&OffendingInstruction,Window->MainWindow,
			&MainWindowGetSampleLeftCopy,&MainWindowGetSampleRightCopy,
			&MainWindowGetSampleMonoCopy,&MainWindowGetWaveTableFrameCount,
			&MainWindowGetWaveTableTableCount,&MainWindowGetWaveTableArray);
		if (OtherError != eEvalNoError)
			{
				char*					FuncNameString;
				MyBoolean			SuccessFlag;

				/* present error message */
				SuccessFlag = False;
				if (GetOpcodeFromPcode(FuncCode) == ErrorOpcode)
					{
						/* our function, not a library function */
						FuncNameString = StringToBlockCopy("<anonymous>");
					}
				 else
					{
						/* it is a library function, so look it up */
						FuncNameString = StringToBlockCopy(GetFunctionName(
							GetFunctionFromOpcode(Window->CodeCenter,ErrorOpcode)));
					}
				if (FuncNameString != NIL)
					{
						char*					Key;

						Key = StringToBlockCopy("_");
						if (Key != NIL)
							{
								char*					BaseMessage;

								BaseMessage = StringFromRaw("Error in function _, instruction _:  _");
								if (BaseMessage != NIL)
									{
										char*					FixedMessage1;

										FixedMessage1 = ReplaceBlockCopy(BaseMessage,Key,FuncNameString);
										if (FixedMessage1 != NIL)
											{
												char*					NumberStr;

												NumberStr = IntegerToString(OffendingInstruction);
												if (NumberStr != NIL)
													{
														char*					FixedMessage2;

														FixedMessage2 = ReplaceBlockCopy(FixedMessage1,Key,NumberStr);
														if (FixedMessage2 != NIL)
															{
																AlertHalt(FixedMessage2,GetPcodeErrorMessage(OtherError));
																SuccessFlag = True;
																ReleasePtr(FixedMessage2);
															}
														ReleasePtr(NumberStr);
													}
												ReleasePtr(FixedMessage1);
											}
										ReleasePtr(BaseMessage);
									}
								ReleasePtr(Key);
							}
						ReleasePtr(FuncNameString);
					}
				if (!SuccessFlag)
					{
						AlertHalt("There is not enough memory available to show "
							"the compile error message.",NIL);
					}
				DisposeParamStack(ParamList);
				DisposePcode(FuncCode);
				return;
			}

		/* add new data to window */
		MemoryErrorOccurred = False;
		TextEditAppendLineInteraction(Window->Editor,NIL);
		switch (ReturnType)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CalculatorWindowDoCalculation:  unknown type"));
					break;
				case eBoolean:
					/* prepare value */
					if (GetStackInteger(ParamList,0) != 0)
						{
							Number = StringToBlockCopy("true");
						}
					 else
						{
							Number = StringToBlockCopy("false");
						}
					Statement = StringToBlockCopy("returns boolean:  _");
					/* put value in editor */
				 ScalarMakePoint:
					if (Number != NIL)
						{
							if (Statement != NIL)
								{
									char*				Key;

									Key = StringToBlockCopy("_");
									if (Key != NIL)
										{
											char*				Fixer;

											Fixer = ReplaceBlockCopy(Statement,Key,Number);
											if (Fixer != NIL)
												{
													TextEditAppendLineInteraction(Window->Editor,Fixer);
													ReleasePtr(Fixer);
												}
											 else
												{
													MemoryErrorOccurred = True;
												}
											ReleasePtr(Key);
										}
									 else
										{
											MemoryErrorOccurred = True;
										}
								}
							 else
								{
									MemoryErrorOccurred = True;
								}
						}
					 else
						{
							MemoryErrorOccurred = True;
						}
					/* dispose values */
					if (Number != NIL)
						{
							ReleasePtr(Number);
						}
					if (Statement != NIL)
						{
							ReleasePtr(Statement);
						}
					break;
				case eInteger:
					Number = IntegerToString(GetStackInteger(ParamList,0));
					Statement = StringToBlockCopy("returns integer:  _");
					goto ScalarMakePoint;
				case eFixed:
					Number = LongDoubleToString(
						largefixed2double(GetStackInteger(ParamList,0)),11,1e-9,1e6);
					Statement = StringToBlockCopy("returns fixed:  _");
					goto ScalarMakePoint;
				case eFloat:
					Number = LongDoubleToString(GetStackFloat(ParamList,0),7,1e-4,1e6);
					Statement = StringToBlockCopy("returns float:  _");
					goto ScalarMakePoint;
				case eDouble:
					Number = LongDoubleToString(GetStackLongDouble(ParamList,0),15,1e-4,1e6);
					Statement = StringToBlockCopy("returns double:  _");
					goto ScalarMakePoint;
				case eArrayOfBoolean:
					Statement = StringToBlockCopy("returns array of boolean:");
					goto ArrayMakePoint;
				case eArrayOfInteger:
					Statement = StringToBlockCopy("returns array of integer:");
					goto ArrayMakePoint;
				case eArrayOfFixed:
					Statement = StringToBlockCopy("returns array of fixed:");
					goto ArrayMakePoint;
				case eArrayOfFloat:
					Statement = StringToBlockCopy("returns array of float:");
					goto ArrayMakePoint;
				case eArrayOfDouble:
					/* construct statement */
					Statement = StringToBlockCopy("returns array of double:");
				 ArrayMakePoint:
					if (Statement != NIL)
						{
							void*					Array;

							TextEditAppendLineInteraction(Window->Editor,Statement);
							ReleasePtr(Statement);
							Array = GetStackArray(ParamList,0);
							if (Array == NIL)
								{
									char*					Thang;

									Thang = StringToBlockCopy("NIL");
									if (Thang != NIL)
										{
											TextEditAppendLineInteraction(Window->Editor,Thang);
											ReleasePtr(Thang);
										}
									 else
										{
											MemoryErrorOccurred = True;
										}
								}
							 else
								{
									long					Scan;
									long					ElementSize;
									long					Limit;

									switch (ReturnType)
										{
											case eArrayOfBoolean:
												ElementSize = sizeof(char);
												break;
											case eArrayOfInteger:
												ElementSize = sizeof(long);
												break;
											case eArrayOfFixed:
												ElementSize = sizeof(largefixedsigned);
												break;
											case eArrayOfFloat:
												ElementSize = sizeof(float);
												break;
											case eArrayOfDouble:
												ElementSize = sizeof(double);
												break;
											default:
												EXECUTE(PRERR(ForceAbort,
													"CalculatorWindowDoCalculation:  type filter failure"));
												break;
										}
									Limit = PtrSize((char*)Array) / ElementSize;
									for (Scan = 0; (Scan < Limit) && !MemoryErrorOccurred
										&& !RelinquishCPUJudiciouslyCheckCancel(); Scan += 1)
										{
											char*					LineNum;

											LineNum = IntegerToString(Scan);
											if (LineNum != NIL)
												{
													char*					Data;

													switch (ReturnType)
														{
															case eArrayOfBoolean:
																if (((char*)Array)[Scan])
																	{
																		Data = StringToBlockCopy("true");
																	}
																 else
																	{
																		Data = StringToBlockCopy("false");
																	}
																break;
															case eArrayOfInteger:
																Data = IntegerToString(((long*)Array)[Scan]);
																break;
															case eArrayOfFixed:
																Data = LongDoubleToString(largefixed2double(
																	((largefixedsigned*)Array)[Scan]),11,1e-9,1e6);
																break;
															case eArrayOfFloat:
																Data = LongDoubleToString(((float*)
																	Array)[Scan],7,1e-4,1e6);
																break;
															case eArrayOfDouble:
																Data = LongDoubleToString(((double*)
																	Array)[Scan],15,1e-4,1e6);
																break;
															default:
																EXECUTE(PRERR(ForceAbort,
																	"CalculatorWindowDoCalculation:  type filter failure"));
																break;
														}
													if (Data != NIL)
														{
															char*				LineNumCopy;

															if (PtrSize(LineNum) > 16 - 1)
																{
																	LineNumCopy = CopyPtr(LineNum);
																}
															 else
																{
																	long				Index;

																	LineNumCopy = AllocPtrCanFail(16,"LineNumCopy");
																	if (LineNumCopy != NIL)
																		{
																			for (Index = 0; Index < 16; Index += 1)
																				{
																					LineNumCopy[Index] = 32;
																				}
																			CopyData(LineNum,LineNumCopy,PtrSize(LineNum));
																			LineNumCopy[PtrSize(LineNum)] = ':';
																		}
																}
															if (LineNumCopy != NIL)
																{
																	char*					Total;

																	Total = ConcatBlockCopy(LineNumCopy,Data);
																	if (Total != NIL)
																		{
																			TextEditAppendLineInteraction(Window->Editor,Total);
																			ReleasePtr(Total);
																		}
																	 else
																		{
																			MemoryErrorOccurred = True;
																		}
																	ReleasePtr(LineNumCopy);
																}
															 else
																{
																	MemoryErrorOccurred = True;
																}
															ReleasePtr(Data);
														}
													 else
														{
															MemoryErrorOccurred = True;
														}
													ReleasePtr(LineNum);
												}
											 else
												{
													MemoryErrorOccurred = True;
												}
										}
								}
						}
					 else
						{
							MemoryErrorOccurred = True;
						}
					break;
			}
		/* add final terminating thing */
		TextEditAppendLineInteraction(Window->Editor,NIL);
		TextEditShowSelection(Window->Editor);
		if (MemoryErrorOccurred)
			{
				AlertHalt("There is not enough memory available to completely display the "
					"results of evaluating the function.",NIL);
			}

		/* remember new last line */
		Window->LastLine = GetTextEditNumLines(Window->Editor) - 1;

		/* dispose of stuff */
		DisposeParamStack(ParamList);
		DisposePcode(FuncCode);
	}
