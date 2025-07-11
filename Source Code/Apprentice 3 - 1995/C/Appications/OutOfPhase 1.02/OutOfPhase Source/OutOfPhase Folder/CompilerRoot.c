/* CompilerRoot.c */
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

#include "CompilerRoot.h"
#include "Memory.h"
#include "CompilerScanner.h"
#include "TrashTracker.h"
#include "CompilerParser.h"
#include "SymbolTable.h"
#include "SymbolTableEntry.h"
#include "ASTExpression.h"
#include "PromotableTypeCheck.h"
#include "SymbolList.h"
#include "CodeCenter.h"
#include "FunctionCode.h"
#include "PeepholeOptimizer.h"
#include "DataMunging.h"
#include "ASTExpressionList.h"


#define OPAREN "("
#define CPAREN ")"
#define OBRACKET "["
#define CBRACKET "]"




/* auxilliary routine for loading keywords */
static void				LoadKeywordsIntoScanner(ScannerRec* Scanner)
	{
		CheckPtrExistence(Scanner);
		AddKeywordToScanner(Scanner,"func",eKeywordFunc);
		AddKeywordToScanner(Scanner,"proto",eKeywordProto);
		AddKeywordToScanner(Scanner,"void",eKeywordVoid);
		AddKeywordToScanner(Scanner,"bool",eKeywordBool);
		AddKeywordToScanner(Scanner,"int",eKeywordInt);
		AddKeywordToScanner(Scanner,"single",eKeywordSingle);
		AddKeywordToScanner(Scanner,"double",eKeywordDouble);
		AddKeywordToScanner(Scanner,"fixed",eKeywordFixed);
		AddKeywordToScanner(Scanner,"boolarray",eKeywordBoolarray);
		AddKeywordToScanner(Scanner,"intarray",eKeywordIntarray);
		AddKeywordToScanner(Scanner,"singlearray",eKeywordSinglearray);
		AddKeywordToScanner(Scanner,"doublearray",eKeywordDoublearray);
		AddKeywordToScanner(Scanner,"fixedarray",eKeywordFixedarray);
		AddKeywordToScanner(Scanner,"var",eKeywordVar);
		AddKeywordToScanner(Scanner,"if",eKeywordIf);
		AddKeywordToScanner(Scanner,"while",eKeywordWhile);
		AddKeywordToScanner(Scanner,"do",eKeywordDo);
		AddKeywordToScanner(Scanner,"until",eKeywordUntil);
		AddKeywordToScanner(Scanner,"set",eKeywordSet);
		AddKeywordToScanner(Scanner,"resize",eKeywordResize);
		AddKeywordToScanner(Scanner,"to",eKeywordTo);
		AddKeywordToScanner(Scanner,"error",eKeywordError);
		AddKeywordToScanner(Scanner,"resumable",eKeywordResumable);
		AddKeywordToScanner(Scanner,"not",eKeywordNot);
		AddKeywordToScanner(Scanner,"sin",eKeywordSin);
		AddKeywordToScanner(Scanner,"cos",eKeywordCos);
		AddKeywordToScanner(Scanner,"tan",eKeywordTan);
		AddKeywordToScanner(Scanner,"asin",eKeywordAsin);
		AddKeywordToScanner(Scanner,"acos",eKeywordAcos);
		AddKeywordToScanner(Scanner,"atan",eKeywordAtan);
		AddKeywordToScanner(Scanner,"ln",eKeywordLn);
		AddKeywordToScanner(Scanner,"exp",eKeywordExp);
		AddKeywordToScanner(Scanner,"sqr",eKeywordSqr);
		AddKeywordToScanner(Scanner,"sqrt",eKeywordSqrt);
		AddKeywordToScanner(Scanner,"abs",eKeywordAbs);
		AddKeywordToScanner(Scanner,"neg",eKeywordNeg);
		AddKeywordToScanner(Scanner,"sign",eKeywordSign);
		AddKeywordToScanner(Scanner,"length",eKeywordLength);
		AddKeywordToScanner(Scanner,"pi",eKeywordPi);
		AddKeywordToScanner(Scanner,"true",eKeywordTrue);
		AddKeywordToScanner(Scanner,"false",eKeywordFalse);
		AddKeywordToScanner(Scanner,"then",eKeywordThen);
		AddKeywordToScanner(Scanner,"else",eKeywordElse);
		AddKeywordToScanner(Scanner,"elseif",eKeywordElseif);
		AddKeywordToScanner(Scanner,"and",eKeywordAnd);
		AddKeywordToScanner(Scanner,"or",eKeywordOr);
		AddKeywordToScanner(Scanner,"xor",eKeywordXor);
		AddKeywordToScanner(Scanner,"div",eKeywordDiv);
		AddKeywordToScanner(Scanner,"mod",eKeywordMod);
		AddKeywordToScanner(Scanner,"getsampleleft",eKeywordGetsampleleft);
		AddKeywordToScanner(Scanner,"getsampleright",eKeywordGetsampleright);
		AddKeywordToScanner(Scanner,"getsample",eKeywordGetsample);
		AddKeywordToScanner(Scanner,"getwavenumframes",eKeywordGetwavenumframes);
		AddKeywordToScanner(Scanner,"getwavenumtables",eKeywordGetwavenumtables);
		AddKeywordToScanner(Scanner,"getwavedata",eKeywordGetwavedata);
	}




/* compile a module.  a module is a text block with a series of function definitions. */
/* if compilation succeeds, the functions are added to the CodeCenter object. */
/* the text data is NOT altered. */
CompileErrors			CompileModule(long* ErrorLineNumber, char* TextData, void* Signature,
										struct CodeCenterRec* CodeCenter)
	{
		TrashTrackRec*	TheTrashCan;
		ScannerRec*			TheScanner;
		MyBoolean				LoopFlag;
		SymbolTableRec*	TheSymbolTable;
		CompileErrors		FinalErrorThing;

		EXECUTE(*ErrorLineNumber = 0x81818181;)
		CheckPtrExistence(TextData);
		CheckPtrExistence(CodeCenter);
		FinalErrorThing = eCompileNoError;

		TheTrashCan = NewTrashTracker();
		if (TheTrashCan == NIL)
			{
			 NoMemoryPoint1:
				if (FinalErrorThing != eCompileNoError)
					{
						/* since we're aborting, dump any functions we've already created */
						FlushModulesCompiledFunctions(CodeCenter,Signature);
					}
				*ErrorLineNumber = 1;
				return eCompileOutOfMemory;
			}

		TheScanner = NewScanner(TheTrashCan,TextData);
		if (TheScanner == NIL)
			{
			 NoMemoryPoint2:
				DisposeTrashTracker(TheTrashCan);
				goto NoMemoryPoint1;
			}
		LoadKeywordsIntoScanner(TheScanner);

		TheSymbolTable = NewSymbolTable(TheTrashCan);
		if (TheSymbolTable == NIL)
			{
				goto NoMemoryPoint2;
			}

		/* loop until there are no more things to parse */
		LoopFlag = True;
		while (LoopFlag)
			{
				TokenRec*				Token;
				long						InitialLineNumberOfForm;

				Token = GetNextToken(TheScanner);
				if (Token == NIL)
					{
						goto NoMemoryPoint2;
					}
				InitialLineNumberOfForm = GetCurrentLineNumber(TheScanner);
				if (GetTokenType(Token) == eTokenEndOfInput)
					{
						/* no more functions to parse, so stop */
						LoopFlag = False;
					}
				 else
					{
						CompileErrors				Error;
						SymbolRec*					SymbolTableEntryForForm;
						ASTExpressionRec*		FunctionBodyRecord;

						/* parse the function */
						UngetToken(TheScanner,Token);
						Error = ParseForm(&SymbolTableEntryForForm,&FunctionBodyRecord,
							TheScanner,TheSymbolTable,TheTrashCan,ErrorLineNumber);
						if (Error != eCompileNoError)
							{
								ERROR(*ErrorLineNumber == 0x81818181,PRERR(AllowResume,
									"CompileModule:  error line number not set properly"));
								FinalErrorThing = Error;
								goto ExitErrorPoint;
							}
						/* SymbolTableEntryForForm will be the symbol table entry that */
						/* was added to the symbol table.  FunctionBodyRecord is either */
						/* an expression for a function or NIL if it was a prototype */

						/* if no error occurred, then generate code */
						if (FunctionBodyRecord != NIL)
							{
								DataTypes						ResultingType;
								SymbolListRec*			FormalArgumentListScan;
								long								ArgumentIndex;
								PcodeRec*						TheFunctionCode;
								long								StackDepth;
								FuncCodeRec*				TheWholeFunctionThing;
								long								ReturnValueLocation;

								/* only generate code for real functions */

								/* step 0:  make sure it hasn't been created yet */
								if (CodeCenterHaveThisFunction(CodeCenter,GetSymbolName(
									SymbolTableEntryForForm),PtrSize(GetSymbolName(
									SymbolTableEntryForForm))))
									{
										FinalErrorThing = eCompileMultiplyDeclaredFunction;
										goto ExitErrorPoint;
									}

								/* step 1:  do type checking */
								Error = TypeCheckExpression(&ResultingType,FunctionBodyRecord,
									ErrorLineNumber,TheTrashCan);
								if (Error != eCompileNoError)
									{
										ERROR(*ErrorLineNumber == 0x81818181,PRERR(AllowResume,
											"CompileModule:  error line number not set properly"));
										FinalErrorThing = Error;
										goto ExitErrorPoint;
									}
								/* check to see that resulting type matches declared type */
								if (!CanRightBeMadeToMatchLeft(GetSymbolFunctionReturnType(
									SymbolTableEntryForForm),ResultingType))
									{
										*ErrorLineNumber = InitialLineNumberOfForm;
										FinalErrorThing = eCompileTypeMismatch;
										goto ExitErrorPoint;
									}
								/* if it has to be promoted, then promote it */
								if (MustRightBePromotedToLeft(GetSymbolFunctionReturnType(
									SymbolTableEntryForForm),ResultingType))
									{
										ASTExpressionRec*		ReplacementExpr;

										/* insert promotion operator above expression */
										ReplacementExpr = PromoteTheExpression(ResultingType,
											GetSymbolFunctionReturnType(SymbolTableEntryForForm),
											FunctionBodyRecord,InitialLineNumberOfForm,TheTrashCan);
										if (ReplacementExpr == NIL)
											{
												*ErrorLineNumber = InitialLineNumberOfForm;
												FinalErrorThing = eCompileOutOfMemory;
												goto ExitErrorPoint;
											}
										FunctionBodyRecord = ReplacementExpr;
										/* sanity check */
										Error = TypeCheckExpression(&ResultingType,FunctionBodyRecord,
											ErrorLineNumber,TheTrashCan);
										ERROR((Error != eCompileNoError),PRERR(ForceAbort,
											"CompileModule:  type promotion caused failure"));
										ERROR(ResultingType != GetSymbolFunctionReturnType(
											SymbolTableEntryForForm),PRERR(ForceAbort,"CompileModule:  "
											"after type promotion, types are no longer the same"));
									}

								/* step 2:  do code generation */
								/* calling conventions:  */
								/*  - leave a space for the return value */
								/*  - push the arguments */
								/*  - jsr pushes the return address */
								/* thus, upon entry, Stack[0] will be the return address */
								/* and Stack[-1] will be the rightmost argument */
								StackDepth = 1;
								ReturnValueLocation = StackDepth; /* remember return value location */
								ArgumentIndex = 0;
								FormalArgumentListScan = GetSymbolFunctionArgList(SymbolTableEntryForForm);
								while (FormalArgumentListScan != NIL)
									{
										SymbolRec*					TheFormalArg;

										TheFormalArg = GetFirstFromSymbolList(FormalArgumentListScan);
										StackDepth += 1; /* allocate first */
										SetSymbolVariableStackLocation(TheFormalArg,StackDepth); /* remember */
										ArgumentIndex += 1;
										FormalArgumentListScan = GetRestListFromSymbolList(
											FormalArgumentListScan);
									}
								/* reserve return address spot */
								StackDepth += 1;
								/* allocate the function code */
								TheFunctionCode = NewPcode();
								if (TheFunctionCode == NIL)
									{
										goto NoMemoryPoint2;
									}
								if (!CodeGenExpression(TheFunctionCode,&StackDepth,FunctionBodyRecord))
									{
									 NoMemoryPoint3:
										DisposePcode(TheFunctionCode);
										goto NoMemoryPoint2;
									}
								/* 3 extra words for retval, retaddr, and resultofexpr */
								ERROR(StackDepth != ArgumentIndex + 1 + 1 + 1,PRERR(ForceAbort,
									"CompileModule:  stack depth error after evaluating function"));
								/* move the result to the return slot */
								switch (GetExpressionsResultantType(FunctionBodyRecord))
									{
										default:
											EXECUTE(PRERR(ForceAbort,"CompileModule:  unknown type"));
											break;
										case eBoolean:
										case eInteger:
										case eFixed:
											if (!AddPcodeInstruction(TheFunctionCode,
												epStoreIntegerOnStack,NIL))
												{
													goto NoMemoryPoint3;
												}
											break;
										case eFloat:
											if (!AddPcodeInstruction(TheFunctionCode,
												epStoreFloatOnStack,NIL))
												{
													goto NoMemoryPoint3;
												}
											break;
										case eDouble:
											if (!AddPcodeInstruction(TheFunctionCode,
												epStoreDoubleOnStack,NIL))
												{
													goto NoMemoryPoint3;
												}
											break;
										case eArrayOfBoolean:
										case eArrayOfInteger:
										case eArrayOfFloat:
										case eArrayOfDouble:
										case eArrayOfFixed:
											if (!AddPcodeInstruction(TheFunctionCode,
												epStoreArrayOnStack,NIL))
												{
													goto NoMemoryPoint3;
												}
											break;
									}
								/* where to put it?  well, if there are no arguments, then */
								/* Stack[0] = the value; Stack[-1] = return address, and */
								/* Stack[-2] = the return value */
								if (!AddPcodeOperandInteger(TheFunctionCode,
									ReturnValueLocation - StackDepth))
									{
										goto NoMemoryPoint3;
									}
								/* now pop the result */
								if (!AddPcodeInstruction(TheFunctionCode,epStackPop,NIL))
									{
										goto NoMemoryPoint3;
									}
								StackDepth -= 1;
								/* now, drop all of the parameters from under return address */
								if (ArgumentIndex > 0)
									{
										if (!AddPcodeInstruction(TheFunctionCode,epStackDeallocateUnder,NIL))
											{
												goto NoMemoryPoint3;
											}
										if (!AddPcodeOperandInteger(TheFunctionCode,ArgumentIndex))
											{
												goto NoMemoryPoint3;
											}
										StackDepth -= ArgumentIndex;
									}
								/* now put the return instruction */
								if (!AddPcodeInstruction(TheFunctionCode,epReturnFromSubroutine,NIL))
									{
										goto NoMemoryPoint3;
									}
								StackDepth -= 1;
								ERROR(StackDepth != 1,PRERR(ForceAbort,
									"CompileModule:  stack depth is wrong at end of function"));

								/* step 2.5:  optimize the code */
								OptimizePcode(TheFunctionCode);

								/* step 3:  create the function and save it away */
								TheWholeFunctionThing = NewFunction(GetSymbolName(
									SymbolTableEntryForForm),PtrSize(GetSymbolName(
									SymbolTableEntryForForm)),GetSymbolFunctionArgList(
									SymbolTableEntryForForm),TheFunctionCode,
									GetSymbolFunctionReturnType(SymbolTableEntryForForm));
								if (TheWholeFunctionThing == NIL)
									{
										goto NoMemoryPoint3;
									}
								/* add it to the code center */
								if (!AddFunctionToCodeCenter(CodeCenter,TheWholeFunctionThing,Signature))
									{
									 NoMemoryPoint4:
										DisposeFunction(TheWholeFunctionThing);
										goto NoMemoryPoint3;
									}

								/* wow, all done!  on to the next one */
							}
					}
			}

	 ExitErrorPoint:
		/* clean up the mess */
		DisposeTrashTracker(TheTrashCan);

		if (FinalErrorThing != eCompileNoError)
			{
				/* since we're aborting, dump any functions we've already created */
				FlushModulesCompiledFunctions(CodeCenter,Signature);
			}

		return FinalErrorThing;
	}




/* return a null terminated static string describing the error. */
char*							GetCompileErrorString(CompileErrors Error)
	{
		char*						S;

		switch (Error)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"GetCompileErrorString:  unknown error code"));
					break;
				case eCompileNoError:
					EXECUTE(PRERR(ForceAbort,"GetCompileErrorString:  eCompileNoError called"));
					break;
				case eCompileOutOfMemory:
					S = "Out of memory";
					break;
				case eCompileExpectedFuncOrProto:
					S = "Expected 'func' or 'proto'";
					break;
				case eCompileExpectedFunc:
					S = "Expected 'func'";
					break;
				case eCompileExpectedIdentifier:
					S = "Expected identifier";
					break;
				case eCompileExpectedOpenParen:
					S = "Expected '"OPAREN"'";
					break;
				case eCompileExpectedCloseParen:
					S = "Expected '"CPAREN"'";
					break;
				case eCompileExpectedColon:
					S = "Expected ':'";
					break;
				case eCompileExpectedSemicolon:
					S = "Expected ';'";
					break;
				case eCompileMultiplyDefinedIdentifier:
					S = "Identifier has already been used";
					break;
				case eCompileExpectedTypeSpecifier:
					S = "Expected type specification";
					break;
				case eCompileExpectedExpressionForm:
					S = "Expected expression";
					break;
				case eCompileExpectedColonEqual:
					S = "Expected ':='";
					break;
				case eCompileExpectedTo:
					S = "Expected 'to'";
					break;
				case eCompileExpectedStringLiteral:
					S = "Expected string literal";
					break;
				case eCompileExpectedResumable:
					S = "Expected 'resumable'";
					break;
				case eCompileExpectedWhile:
					S = "Expected 'while'";
					break;
				case eCompileExpectedDo:
					S = "Expected 'do'";
					break;
				case eCompileExpectedUntil:
					S = "Expected 'until'";
					break;
				case eCompileExpectedOpenParenOrEqual:
					S = "Expected '"OPAREN"' or '='";
					break;
				case eCompileExpectedThen:
					S = "Expected 'then'";
					break;
				case eCompileExpectedWhileOrUntil:
					S = "Expected 'while' or 'until'";
					break;
				case eCompileExpectedCommaOrCloseParen:
					S = "Expected ',' or '"CPAREN"'";
					break;
				case eCompileExpectedElseOrElseIf:
					S = "Expected 'else' or 'elseif'";
					break;
				case eCompileExpectedOperatorOrStatement:
					S = "Expected operator or statement";
					break;
				case eCompileExpectedOperand:
					S = "Expected operand";
					break;
				case eCompileIdentifierNotDeclared:
					S = "Identifier hasn't been declared";
					break;
				case eCompileExpectedRightAssociativeOperator:
					S = "Expected right associative operator";
					break;
				case eCompileExpectedOpenBracket:
					S = "Expected '"OBRACKET"'";
					break;
				case eCompileExpectedCloseBracket:
					S = "Expected '"CBRACKET"'";
					break;
				case eCompileExpectedVariable:
					S = "Expected variable";
					break;
				case eCompileExpectedArrayType:
					S = "Expected array type";
					break;
				case eCompileArraySizeSpecMustBeInteger:
					S = "Array size specifier must be integer";
					break;
				case eCompileTypeMismatch:
					S = "Type conflict";
					break;
				case eCompileInvalidLValue:
					S = "Invalid l-value";
					break;
				case eCompileOperandsMustBeScalar:
					S = "Operands must be scalar";
					break;
				case eCompileOperandsMustBeSequencedScalar:
					S = "Operands must be sequenced scalar";
					break;
				case eCompileOperandsMustBeIntegers:
					S = "Operands must be an integer";
					break;
				case eCompileRightOperandMustBeInteger:
					S = "Right operand must be an integer";
					break;
				case eCompileArraySubscriptMustBeInteger:
					S = "Array subscript must be an integer";
					break;
				case eCompileArrayRequiredForSubscription:
					S = "Array required for subscription";
					break;
				case eCompileDoubleRequiredForExponentiation:
					S = "Operands for exponentiation must be doubles";
					break;
				case eCompileArrayRequiredForResize:
					S = "Array required for resize";
					break;
				case eCompileIntegerRequiredForResize:
					S = "Integer required resize array size specifier";
					break;
				case eCompileConditionalMustBeBoolean:
					S = "Conditional expression must boolean";
					break;
				case eCompileTypeMismatchBetweenThenAndElse:
					S = "Type conflict between consequent and alternate arms of conditional";
					break;
				case eCompileErrorNeedsBooleanArg:
					S = "Error directive needs boolean argument";
					break;
				case eCompileFunctionIdentifierRequired:
					S = "Function identifier required";
					break;
				case eCompileArgumentTypeConflict:
					S = "Type conflict between formal and actual arguments";
					break;
				case eCompileWrongNumberOfArgsToFunction:
					S = "Wrong number of arguments to function";
					break;
				case eCompileCantHaveStringLiteralThere:
					S = "String literal is not allowed here";
					break;
				case eCompileMustBeAVariableIdentifier:
					S = "Variable identifier required";
					break;
				case eCompileOperandMustBeBooleanOrInteger:
					S = "Operands must be boolean or integer";
					break;
				case eCompileOperandMustBeDouble:
					S = "Operand must be double";
					break;
				case eCompileArrayRequiredForGetLength:
					S = "Array required for length operator";
					break;
				case eCompileTypeMismatchInAssignment:
					S = "Type conflict between l-value and argument";
					break;
				case eCompileVoidExpressionIsNotAllowed:
					S = "Void expression is not allowed";
					break;
				case eCompileMultiplyDeclaredFunction:
					S = "Function name has already been used";
					break;
				case eCompilePrototypeCantBeLastThingInExprList:
					S = "Prototype can't be the last expression in an expression sequence";
					break;
				case eCompileInputBeyondEndOfFunction:
					S = "Input beyond end of expression";
					break;
				case eCompileArrayConstructionOnScalarType:
					S = "Array constructor applied to scalar variable";
					break;
			}
		return S;
	}




/* compile a special function.  a special function has no function header, but is */
/* simply some code to be executed.  the parameters the code is expecting are provided */
/* in the FuncArray[] and NumParams.  the first parameter is deepest beneath the */
/* top of stack.  the TextData is NOT altered.  if an error occurrs, *FunctionOut */
/* will NOT contain a valid object */
CompileErrors			CompileSpecialFunction(FunctionParamRec FuncArray[], long NumParams,
										long* ErrorLineNumber, DataTypes* ReturnType, char* TextData,
										struct PcodeRec** FunctionOut)
	{
		TrashTrackRec*		TheTrashCan;
		ScannerRec*				TheScanner;
		SymbolTableRec*		TheSymbolTable;
		long							StackDepth;
		long							ReturnAddressIndex;
		CompileErrors			Error;
		ASTExpressionRec*	TheExpressionThang;
		ASTExprListRec*		ListOfExpressions;
		PcodeRec*					TheFunctionCode;
		long							Scan;
		DataTypes					ResultingType;
		TokenRec*					Token;

		EXECUTE(*ErrorLineNumber = 0x81818181;)
		CheckPtrExistence(TextData);

		TheTrashCan = NewTrashTracker();
		if (TheTrashCan == NIL)
			{
			 NoMemoryPoint1:
				*ErrorLineNumber = 1;
				return eCompileOutOfMemory;
			}

		TheScanner = NewScanner(TheTrashCan,TextData);
		if (TheScanner == NIL)
			{
			 NoMemoryPoint2:
				DisposeTrashTracker(TheTrashCan);
				goto NoMemoryPoint1;
			}
		LoadKeywordsIntoScanner(TheScanner);

		TheSymbolTable = NewSymbolTable(TheTrashCan);
		if (TheSymbolTable == NIL)
			{
				goto NoMemoryPoint2;
			}

		/* build parameters into symbol table */
		StackDepth = 1;
		ReturnAddressIndex = StackDepth;
		for (Scan = 0; Scan < NumParams; Scan += 1)
			{
				SymbolRec*				TheParameter;

				TheParameter = NewSymbol(TheTrashCan,FuncArray[Scan].ParameterName,
					StrLen(FuncArray[Scan].ParameterName));
				if (TheParameter == NIL)
					{
						goto NoMemoryPoint2;
					}
				SymbolBecomeVariable(TheParameter,FuncArray[Scan].ParameterType);
				/* allocate stack slot */
				StackDepth += 1;
				SetSymbolVariableStackLocation(TheParameter,StackDepth);
				switch (AddSymbolToTable(TheSymbolTable,TheParameter))
					{
						default:
							EXECUTE(PRERR(ForceAbort,"CompileSpecialFunction:  unknown "
								"return code from AddSymbolToTable"));
							break;
						case eAddSymbolNoErr:
							break;
						case eAddSymbolAlreadyExists:
							EXECUTE(PRERR(ForceAbort,"CompileSpecialFunction:  "
								"eAddSymbolAlreadyExists was returned from AddSymbolToTable"));
							break;
						case eAddSymbolNoMemory:
							goto NoMemoryPoint2;
					}
			}
		/* fence them off */
		if (!IncrementSymbolTableLevel(TheSymbolTable))
			{
				goto NoMemoryPoint2;
			}

		/* save return address spot */
		/* StackDepth += 1; NO RETURN ADDRESS! */

		Error = ParseExprList(&ListOfExpressions,TheScanner,TheSymbolTable,TheTrashCan,
			ErrorLineNumber);
		/* compile the thing */
		if (Error != eCompileNoError)
			{
				ERROR(*ErrorLineNumber == 0x81818181,PRERR(ForceAbort,
					"CompileSpecialFunction:  error line number wasn't set properly"));
				DisposeTrashTracker(TheTrashCan);
				return Error;
			}
		TheExpressionThang = NewExprSequence(ListOfExpressions,TheTrashCan,
			GetCurrentLineNumber(TheScanner));
		if (TheExpressionThang == NIL)
			{
				*ErrorLineNumber = GetCurrentLineNumber(TheScanner);
				DisposeTrashTracker(TheTrashCan);
				return eCompileOutOfMemory;
			}

		/* make sure there is nothing after it */
		Token = GetNextToken(TheScanner);
		if (Token == NIL)
			{
				goto NoMemoryPoint2;
			}
		if (GetTokenType(Token) != eTokenEndOfInput)
			{
				*ErrorLineNumber = GetCurrentLineNumber(TheScanner);
				DisposeTrashTracker(TheTrashCan);
				return eCompileInputBeyondEndOfFunction;
			}

		Error = TypeCheckExpression(&ResultingType,TheExpressionThang,
			ErrorLineNumber,TheTrashCan);
		if (Error != eCompileNoError)
			{
				ERROR(*ErrorLineNumber == 0x81818181,PRERR(ForceAbort,
					"CompileSpecialFunction:  error line number wasn't set properly"));
				DisposeTrashTracker(TheTrashCan);
				return Error;
			}

		TheFunctionCode = NewPcode();
		if (TheFunctionCode == NIL)
			{
				goto NoMemoryPoint2;
			}

		if (!CodeGenExpression(TheFunctionCode,&StackDepth,TheExpressionThang))
			{
			 NoMemoryPoint3:
				DisposePcode(TheFunctionCode);
				goto NoMemoryPoint2;
			}

		ERROR(ReturnType == NIL,PRERR(ForceAbort,
			"CompileSpecialFunction:  ReturnType is NIL"));
		*ReturnType = GetExpressionsResultantType(TheExpressionThang);


		/* append epilogue */
		/* 1. store result in return slot */
		/* 2. pop result */
		/* note that we do NOT pop the parameters!!! */

		/* 2 extra words for retval and resultofexpr */
		ERROR(StackDepth != NumParams + 1 + 1,PRERR(ForceAbort,
			"CompileSpecialFunction:  stack depth error after evaluating function"));
		/* move the result to the return slot */
		switch (GetExpressionsResultantType(TheExpressionThang))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CompileSpecialFunction:  unknown type"));
					break;
				case eBoolean:
				case eInteger:
				case eFixed:
					if (!AddPcodeInstruction(TheFunctionCode,epStoreIntegerOnStack,NIL))
						{
							goto NoMemoryPoint3;
						}
					break;
				case eFloat:
					if (!AddPcodeInstruction(TheFunctionCode,epStoreFloatOnStack,NIL))
						{
							goto NoMemoryPoint3;
						}
					break;
				case eDouble:
					if (!AddPcodeInstruction(TheFunctionCode,epStoreDoubleOnStack,NIL))
						{
							goto NoMemoryPoint3;
						}
					break;
				case eArrayOfBoolean:
				case eArrayOfInteger:
				case eArrayOfFloat:
				case eArrayOfDouble:
				case eArrayOfFixed:
					if (!AddPcodeInstruction(TheFunctionCode,epStoreArrayOnStack,NIL))
						{
							goto NoMemoryPoint3;
						}
					break;
			}
		/* where to put it?  well, if there are no arguments, then */
		/* Stack[0] = the value; Stack[-1] = return address, and */
		/* Stack[-2] = the return value */
		if (!AddPcodeOperandInteger(TheFunctionCode,ReturnAddressIndex - StackDepth))
			{
				goto NoMemoryPoint3;
			}
		/* now pop the result */
		if (!AddPcodeInstruction(TheFunctionCode,epStackPop,NIL))
			{
				goto NoMemoryPoint3;
			}
		StackDepth -= 1;
		/* now put the return instruction */
		if (!AddPcodeInstruction(TheFunctionCode,epReturnFromSubroutine,NIL))
			{
				goto NoMemoryPoint3;
			}
		/* magically, there is no return address, since the root function call */
		/* is detected by the pcode evaluator and doesn't need a return address */
		ERROR(StackDepth != NumParams + 1,PRERR(ForceAbort,
			"CompileSpecialFunction:  stack depth is wrong at end of function"));

		/* optimize stupid things away */
		OptimizePcode(TheFunctionCode);


		/* it worked, so return the dang thing */
		*FunctionOut = TheFunctionCode;
		DisposeTrashTracker(TheTrashCan);
		return eCompileNoError;
	}
