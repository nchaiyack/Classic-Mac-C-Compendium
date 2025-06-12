/* Audit.c */

#include "Audit.h"
#include "MiscInfo.h"


#ifdef AUDIT

typedef void *va_list;
#define __va(arg)				&arg + 1
#define va_start(p, arg)		p = __va(arg)
#define va_arg(p, type)			*(* (type **) &p)++
#define va_end(p)

static short	AuditRefNum;

#if __option(mc68020)
	#define CodeFor68020
#else
	#define CodeFor68000
#endif
#pragma options(!mc68020) /* this code works no matter what */


void		INITAUDIT(void)
	{
		char		FileName[] = {"\p Audit Trail"};

		FSDelete((unsigned char*)FileName,0);
		ERROR(Create((unsigned char*)FileName,0,AUDITCREATOR,'TEXT') != noErr,
			PRERR(ForceAbort,"Audit_Init couldn't create audit trail file."));
		ERROR(FSOpen((unsigned char*)FileName,0,&AuditRefNum) != noErr,
			PRERR(ForceAbort,"Audit_Init couldn't open audit trail file for writing."));
	}

void		ENDAUDIT(void)
	{
		FSClose(AuditRefNum);
	}

static void	FlushBuffer(char Buffer[1024], short* BufPtr, MyBoolean FlushAllFlag)
	{
		char*		Place;
		long		Count;

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


void		AHEXDUMP(char* Ptr, long NumBytes)
	{
		while (NumBytes >= 8)
			{
				AuditPrint("%xc %xc %xc %xc %xc %xc %xc %xc",
					(uchar)*Ptr,(uchar)*(Ptr+1),(uchar)*(Ptr+2),(uchar)*(Ptr+3),
					(uchar)*(Ptr+4),(uchar)*(Ptr+5),(uchar)*(Ptr+6),(uchar)*(Ptr+7));
				NumBytes -= 8;
				Ptr += 8;
			}
		switch (NumBytes)
			{
				case 1:
					AuditPrint("%xc",(uchar)*Ptr);
					break;
				case 2:
					AuditPrint("%xc %xc",(uchar)*Ptr,(uchar)*(Ptr+1));
					break;
				case 3:
					AuditPrint("%xc %xc %xc",
						(uchar)*Ptr,(uchar)*(Ptr+1),(uchar)*(Ptr+2));
					break;
				case 4:
					AuditPrint("%xc %xc %xc %xc",
						(uchar)*Ptr,(uchar)*(Ptr+1),(uchar)*(Ptr+2),(uchar)*(Ptr+3));
					break;
				case 5:
					AuditPrint("%xc %xc %xc %xc %xc",
						(uchar)*Ptr,(uchar)*(Ptr+1),(uchar)*(Ptr+2),(uchar)*(Ptr+3),
						(uchar)*(Ptr+4));
					break;
				case 6:
					AuditPrint("%xc %xc %xc %xc %xc %xc",
						(uchar)*Ptr,(uchar)*(Ptr+1),(uchar)*(Ptr+2),(uchar)*(Ptr+3),
						(uchar)*(Ptr+4),(uchar)*(Ptr+5));
					break;
				case 7:
					AuditPrint("%xc %xc %xc %xc %xc %xc %xc",
						(uchar)*Ptr,(uchar)*(Ptr+1),(uchar)*(Ptr+2),(uchar)*(Ptr+3),
						(uchar)*(Ptr+4),(uchar)*(Ptr+5),(uchar)*(Ptr+6));
					break;
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
void		AuditPrint(char* Str,...)
	{
		va_list			pa;
		char				Buffer[1024];
		short				BufPtr;
		static char	Hex[16] = {'0','1','2','3','4','5','6','7',
									'8','9','a','b','c','d','e','f'};
		short				VRefNum;

		BufPtr = 0;
		va_start(pa,Str);
		while (*Str != 0)
			{
				if (*Str == '%')
					{
						MyBoolean		HexFlag;

						Str += 1;
						if (HexFlag = (*Str == 'x'))
							{
								Str += 1;
							}
						switch (*Str)
							{
								case 'l':
									{
										long	Num;

										Num = va_arg(pa,long);
										if (HexFlag)	
											{
												char		Buf[9];
												short		Count;
										
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
												char		Buf[16];
												short		BPtr;
												short		Scan;

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
														Buf[--BPtr] = 0;
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
										short Num;

										Num = va_arg(pa,short);
										if (HexFlag)
											{
												char		Buf[5];
												short		Count;

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
														char		Apl[] = "-32768";
														short		Scan;
	
														for (Scan = 0; Apl[Scan] != 0; Scan += 1)
															{
																Buffer[BufPtr++] = Apl[Scan];
															}
													}
												 else
													{
														char		Buf[8];
														short		BPtr;
														short		Scan;
	
														if (Num < 0)
															{
																Buffer[BufPtr++] = '-';
																Num = -Num;
															}
														BPtr = 8;
														Buf[--BPtr] = 0;
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
										char	 Num;

										Num = ((va_arg(pa,short)) >> 0) & 0x00ff;
										if (HexFlag)
											{
												char		Buf[3];
												short		Count;

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
														char		Apl[] = "-128";
														short		Scan;
	
														for (Scan = 0; Apl[Scan] != 0; Scan += 1)
															{
																Buffer[BufPtr++] = Apl[Scan];
															}
													}
												 else
													{
														char		Buf[4];
														short		BPtr;
														short		Scan;
	
														if (Num < 0)
															{
																Buffer[BufPtr++] = '-';
																Num = -Num;
															}
														BPtr = 4;
														Buf[--BPtr] = 0;
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
												char*		Strp;

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
												char*			Temp;
												short			Scan;

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
												MyBoolean	Num;
												char*			Strptr;
												short			Cnt;

												Num = va_arg(pa,short);
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
												short		Cnt;
												char		Msg[] = "???";

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
		GetVRefNum(AuditRefNum,&VRefNum);
//		FlushVol("\p",VRefNum);
	}

#ifdef CodeFor68020
	#pragma options(mc68020) /* turn it back on if necessary */
#endif

#endif
