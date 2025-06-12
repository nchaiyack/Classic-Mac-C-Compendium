/* Audit.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#pragma options(pack_enums)
#include <Files.h>
#pragma options(!pack_enums)


#if AUDIT

typedef void *va_list;
#define __va(arg)  &arg + 1
#define va_start(p,arg)  p = __va(arg)
#define va_arg(p, type)  *(*(type**)&p)++
#define va_end(p)

static short				AuditRefNum;
static MyBoolean		AuditInitialized = False;

static char					Hex[16] = {'0','1','2','3','4','5','6','7',
											'8','9','a','b','c','d','e','f'};

#pragma options(!mc68020) /* this code works no matter what */


static void					INITAUDIT(void)
	{
		unsigned char			FileName[] = {"\p!!Audit Trail"};

		FSDelete(FileName,0);
		ERROR(Create(FileName,0,AUDITCREATOR,'TEXT') != noErr,
			PRERR(ForceAbort,"Audit_Init couldn't create audit trail file."));
		ERROR(FSOpen(FileName,0,&AuditRefNum) != noErr,
			PRERR(ForceAbort,"Audit_Init couldn't open audit trail file for writing."));
		AuditInitialized = True;
	}


void								ENDAUDIT(void)
	{
		FSClose(AuditRefNum);
	}


static void					FlushBuffer(char Buffer[1024], short* BufPtr, MyBoolean FlushAllFlag)
	{
		char*							Place;
		long							Count;

		if ((*BufPtr >= 512) || (FlushAllFlag))
			{
				Place = Buffer;
				while (*BufPtr != 0)
					{
						Count = *BufPtr;
						FSWrite(AuditRefNum,&Count,Place);
						*BufPtr -= Count;
						Place += Count;
					}
			}
	}


#define COLUMNS (16)
void								AHEXDUMP(char* Ptr, long NumBytes)
	{
		while (NumBytes > 0)
			{
				int								Scan;
				char							Buffer[1 + 3*COLUMNS + 1 + COLUMNS + 1];
				short							Index;

				Index = 0;
				Buffer[Index++] = 32;
				for (Scan = 0; Scan < COLUMNS; Scan += 1)
					{
						if (NumBytes > Scan)
							{
								Buffer[Index++] = Hex[(((unsigned char*)Ptr)[Scan] >> 4) & 0x0f];
								Buffer[Index++] = Hex[(((unsigned char*)Ptr)[Scan]) & 0x0f];
								Buffer[Index++] = 32;
							}
						 else
							{
								Buffer[Index++] = 32;
								Buffer[Index++] = 32;
								Buffer[Index++] = 32;
							}
					}
				Buffer[Index++] = 32;
				for (Scan = 0; Scan < COLUMNS; Scan += 1)
					{
						if (NumBytes > Scan)
							{
								if ((((unsigned char*)Ptr)[Scan] >= 32)
									&& (((unsigned char*)Ptr)[Scan] <= 126))
									{
										Buffer[Index++] = ((unsigned char*)Ptr)[Scan];
									}
								 else
									{
										Buffer[Index++] = '.';
									}
							}
						 else
							{
								Buffer[Index++] = 32;
							}
					}
				Buffer[Index++] = 13;
				FlushBuffer(Buffer,&Index,True/*force write*/);
				Ptr += COLUMNS;
				NumBytes -= COLUMNS;
			}
	}


/* this prints a string in the same way that printf does.  it accepts these options: */
/* %s = decimal signed short */
/* $xs = hexadecimal short */
/* %l = decimal signed long */
/* %xl = hexadecimal long */
/* %b = boolean from short */
/* %c = decimal signed char */
/* %xc = hexadecimal char */
/* %t = C String (text) */
/* %p = Pascal string */
/* %r = Reference (a pointer) */
void								AuditPrint(char* Str,...)
	{
		va_list						pa;
		char							Buffer[1024];
		short							BufPtr;
		short							VRefNum;

		if (!AuditInitialized)
			{
				INITAUDIT();
			}
		BufPtr = 0;
		va_start(pa,Str);
		while (*Str != 0)
			{
				if (*Str == '%')
					{
						MyBoolean				HexFlag;

						Str += 1;
						HexFlag = (*Str == 'x');
						if (HexFlag)
							{
								Str += 1;
							}
						switch (*Str)
							{
								case 'r':
									HexFlag = 1;
									goto PrintLong; /* sizeof(pointer) == sizeof(long) on 68000 */
								case 'l':
									{
										unsigned long		Num;

									 PrintLong:
										Num = va_arg(pa,long);
										if (HexFlag)	
											{
												char						Buf[9];
												short						Count;
										
												for (Count = 8; Count >= 1; Count -= 1)
													{
														Buf[Count] = Hex[Num & 0x0000000f];
														Num = Num >> 4;
													}
												Buf[0] = '$';
												for (Count = 0; Count < 9; Count += 1)
													{
														Buffer[BufPtr++] = Buf[Count];
													}
											}
										 else
											{
												char						Buf[16];
												short						BPtr;
												short						Scan;

												if (Num == -2147483648)
													{
														char		Apl[] = "-2147483648";
														short		Scan;

														for (Scan = 0; Apl[Scan] != 0; Scan += 1)
															{
																Buffer[BufPtr++] = Apl[Scan];
															}
													}
												 else
													{
														if (Num < 0)
															{
																Buffer[BufPtr++] = '-';
																Num = -Num;
															}
														BPtr = 16;
														do
															{
																Buf[--BPtr] = (Num % 10) + '0';
																Num = Num / 10;
															} while (Num != 0);
														for (Scan = BPtr; Scan < 16; Scan += 1)
															{
																Buffer[BufPtr++] = Buf[Scan];
															}
													}
											}
									}
									Str += 1;
									break;
								case 's':
									{
										unsigned short		Num;

										Num = va_arg(pa,short);
										if (HexFlag)
											{
												char						Buf[5];
												short						Count;

												for (Count = 4; Count >= 1; Count -= 1)
													{
														Buf[Count] = Hex[Num & 0x000f];
														Num = Num >> 4;
													}
												Buf[0] = '$';
												for (Count = 0; Count < 5; Count += 1)
													{
														Buffer[BufPtr++] = Buf[Count];
													}
											}
										 else
											{
												if (Num == -32768)
													{
														char						Apl[] = "-32768";
														short						Scan;
	
														for (Scan = 0; Apl[Scan] != 0; Scan += 1)
															{
																Buffer[BufPtr++] = Apl[Scan];
															}
													}
												 else
													{
														char						Buf[8];
														short						BPtr;
														short						Scan;
	
														if (Num < 0)
															{
																Buffer[BufPtr++] = '-';
																Num = -Num;
															}
														BPtr = 8;
														do
															{
																Buf[--BPtr] = (Num % 10) + '0';
																Num = Num / 10;
															} while (Num != 0);
														for (Scan = BPtr; Scan < 8; Scan += 1)
															{
																Buffer[BufPtr++] = Buf[Scan];
															}
													}
											}
									}
									Str += 1;
									break;
								case 'c':
									{
										unsigned char		Num;

										Num = (va_arg(pa,short)) & 0x00ff;
										if (HexFlag)
											{
												char						Buf[3];
												short						Count;

												for (Count = 2; Count >= 1; Count -= 1)
													{
														Buf[Count] = Hex[Num & 0x0f];
														Num = Num >> 4;
													}
												Buf[0] = '$';
												for (Count = 0; Count < 3; Count += 1)
													{
														Buffer[BufPtr++] = Buf[Count];
													}
											}
										 else
											{
												if (Num == -128)
													{
														char						Apl[] = "-128";
														short						Scan;
	
														for (Scan = 0; Apl[Scan] != 0; Scan += 1)
															{
																Buffer[BufPtr++] = Apl[Scan];
															}
													}
												 else
													{
														char						Buf[4];
														short						BPtr;
														short						Scan;
	
														if (Num < 0)
															{
																Buffer[BufPtr++] = '-';
																Num = -Num;
															}
														BPtr = 4;
														do
															{
																Buf[--BPtr] = (Num % 10) + '0';
																Num = Num / 10;
															} while (Num != 0);
														for (Scan = BPtr; Scan < 4; Scan += 1)
															{
																Buffer[BufPtr++] = Buf[Scan];
															}
													}
											}
											Str += 1;
											break;
										case 't':
											{
												char*						Strp;

												Strp = va_arg(pa,char*);
												while (*Strp != 0)
													{
														Buffer[BufPtr++] = *(Strp++);
														FlushBuffer(Buffer,&BufPtr,False);
													}
											}
											Str += 1;
											break;
										case 'p':
											{
												char*							Temp;
												short							Scan;

												Temp = va_arg(pa,char*);
												for (Scan = 0; Scan != Temp[0]; Scan += 1)
													{
														Buffer[BufPtr++] = Temp[Scan+1];
													}
											}
											Str += 1;
											break;
										case 'b':
											{
												MyBoolean				Num;
												char*						Strptr;
												short						Cnt;

												Num = va_arg(pa,MyBoolean);
												if (Num)
													{
														Strptr = "True";
													}
												 else
													{
														Strptr = "False";
													}
												for (Cnt = 0; Strptr[Cnt] != 0; Cnt += 1)
													{
														Buffer[BufPtr++] = Strptr[Cnt];
													}
											}
											Str += 1;
											break;
										default:
											{
												short						Cnt;
												char						Msg[] = "???";

												for (Cnt = 0; Msg[Cnt] != 0; Cnt += 1)
													{
														Buffer[BufPtr++] = Msg[Cnt];
													}
											}
											Str += 1;
											break;
									}
							}
					}
				 else
					{
						Buffer[BufPtr++] = *(Str++);
					}
				FlushBuffer(Buffer,&BufPtr,False);
			}
		Buffer[BufPtr++] = 0x0d;
		FlushBuffer(Buffer,&BufPtr,True);
#if AUDITFLUSHING
		GetVRefNum(AuditRefNum,&VRefNum);
		FlushVol("\p",VRefNum);
#endif
	}

#endif
