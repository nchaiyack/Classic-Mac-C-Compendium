{	Coroutines for THINK Pascal	}
{	File: Coroutines	, version 1.1 }
{	Copyright � 1991-1992 Olaf Titz (s_titz@ira.uka.de)	}

{	This program is free software; you can redistribute it and/or modify	}
{	it under the terms of the GNU General Public License as published by	}
{	the Free Software Foundation; either version 2 of the License, or	}
{	(at your option) any later version.	}

{	This program is distributed in the hope that it will be useful,	}
{	but WITHOUT ANY WARRANTY; without even the implied warranty of	}
{	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	}
{	GNU General Public License for more details.	}

{	You should have received a copy of the GNU General Public License	}
{	along with this program; if not, write to the Free Software	}
{	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.	}

unit Coroutines;

interface

var
	GlobalProc: Handle;	{�default ' main' coroutine }
{ A coroutine may exit. Then it gets transfered to GlobalProc and the }
{ workspace deallocated. }

procedure InitCoroutines;
{ to be called at startup }

function NewProcess (proc: ProcPtr; size: integer): Handle;
{ generate a coroutine, allocate workspace }

procedure Transfer (source, dest: Handle);
{ Switch from - to }

procedure ExitCoroutines;
{ to be called on exit }

{ To compile this unit you need to set a compiler option 'stackcheck' to either true }
{ or false. The former case will perform a regular stack checking like the system's, }
{ giving error 28 on overflow; the latter will disable stack checking as well as the }
{ default stack checking. Modules using Coroutines and doing Transfer must be compiled }
{ with debug option off. }

implementation

{$D-,V-,R-}
	{ Don't break me... }

const	{Globals}
	StkLoPtr = $110;
	ApplLimit = $130;
	Lo3Bytes = $31A;
	stacklimit = $A80;	{ Last 4 bytes of ApplScratch }

type
	WSPHndl = ^WSPPtr;
	WSPPtr = ^WSPRecord;
	WSPRecord = array[0..16000] of longint;

{$IFC stackcheck}
var
	StackVBL: VBLTask;
{$ENDC}

procedure _Transfer;
inline
{$IFC stackcheck}
	$42B8, stacklimit,	{ CLR.L	stacklimit						}
{$ENDC}
	$206E, $000C,		{ MOVEA.L  12(A6),A0					}
	$2050,				{ MOVEA.L  (A0),A0					}
	$48D0, $58F8,		{ MOVEM.L  D3-D7/A3-A4/A6,(A0)	}
	$206E, $0008,		{ MOVEA.L  8(A6),A0					}
{$IFC stackcheck}
	$2010,				{ MOVE.L	(A0), D0					}
	$C0B8, Lo3Bytes,	{ AND.L 	Lo3Bytes, D0				}
	$2240,				{ MOVEA.L	D0,A1						}
	$4CD9, $58F8,		{ MOVEM.L  (A1)+,D3-D7/A3-A4/A6	}
	$2E4E,				{ MOVEA.L	A6,A7						}
	$21C9, stacklimit;	{ MOVE.L	A1,stacklimit				}
{$ELSEC}
	$2050,				{ MOVEA.L  (A0),A0					}
	$4CD8, $58F8;		{ MOVEM.L  (A0)+,D3-D7/A3-A4/A6	}
{$ENDC}

procedure Transfer (source, dest: Handle);
	begin				{ LINK		#0,A6						}
		_Transfer;
	end;				{ UNLK		A6							}
						{ MOVEA.L	(A7)+,A0					}
						{ ADDQ.W	#8,A7						}
						{ JMP		(A0)						}


procedure _KillCoroutine;
inline
{$IFC stackcheck}
	$42B8, stacklimit,	{ CLR.L	stacklimit						}
{$ENDC}
	$222E, $0004, 	{ MOVE.L	4(A6),D1					}
	$206E, $0008,		{ MOVEA.L	8(A6),A0					}
{$IFC stackcheck}
	$2010,				{ MOVE.L	(A0), D0					}
	$C0B8, Lo3Bytes,	{ AND.L 	Lo3Bytes, D0				}
	$2240,				{ MOVEA.L	D0,A1						}
{$ELSEC}
	$2250,				{ MOVEA.L  (A0),A1					}
{$ENDC}
	$4CD9, $58F8,		{ MOVEM.L	(A1)+,D3-D7/A3-A4/A6	}
{$IFC stackcheck}
	$2E4E,				{ MOVEA.L	A6,A7						}
	$21C9, stacklimit,	{ MOVE.L	A1,stacklimit				}
{$ENDC}
	$2041,				{ MOVEA.L	D1,A0						}
	$A02A,			{ _HUnlock								}
	$A023;				{ _DisposHandle							}



procedure KillCoroutine (source, dest: Handle);
	begin				{ LINK		#0,A6						}
		_KillCoroutine;
	end;				{ UNLK		A6							}
						{ MOVEA.L	(A7)+,A0					}
						{ ADDQ.W	#8,A7						}
						{ JMP		(A0)						}




function NewProcess (proc: ProcPtr; size: integer): Handle;
	var
		h: WSPHndl;
		nn, i: integer;
	begin
		nn := size div 4;	{ adjust size to longwords }
		if nn < 255 then	{ we need at least 1024 bytes }
			nn := 255;
		size := (nn + 1) * 4;	{ from 0 to nn }
		h := WSPHndl(NewHandle(size));
		if h <> nil then begin
			MoveHHi(Handle(h));
			HLock(Handle(h));
			h^^[nn - 0] := longint(GlobalProc);	{ set wsp parameters }
			h^^[nn - 1] := longint(h);			{ (see docu) }
			h^^[nn - 2] := longint(@KillCoroutine);
			h^^[nn - 5] := longint(Proc);
			h^^[7] := BitAnd(Longint(h^), $00FFFFFF) + (nn - 6) * 4;
		end;
		NewProcess := Handle(h);
	end;


{$IFC stackcheck}

procedure _myStackCheck;
inline
	$BFF8, stacklimit,			{ CMPA.L	stacklimit,A7						}
	$6200, $0006,				{ BHI		_my0								}
	$701C,						{ MOVEQ	#28,D0								}
	$A9C9;						{ _SysError									}
								{_my0:	}

{$A+}
procedure myStackCheck;
	begin
		_myStackCheck;
		StackVBL.vblCount := 1
	end;
{$A-}


{$ENDC stackcheck}

procedure InitCoroutines;
	type
		pl = ^longint;
	var
		ap: pl;
		h: WSPHndl;
		i: longint;
	begin
		new(h);						{ generate a fake WSP handle }
		ap := pl(ApplLimit);
		i := ap^ + 8;
		h^ := WSPPtr(i);			{ pointer to ApplLimit+8... }
		GlobalProc := Handle(h);	{ ...in GlobalProc }
		ap := pl(StkLoPtr);
		ap^ := 0;					{ switch stack checking off }
{$IFC stackcheck}
		ap := pl(stacklimit);
		ap^ := i + 32;
		with StackVBL do begin
			qType := ord(vType);
			vblAddr := @myStackCheck;
			vblCount := 1;
			vblPhase := 0
		end;
		i := VInstall(QElemPtr(@StackVBL));
{$ENDC stackcheck}
	end;


 {$IFC stackcheck}

procedure ExitCoroutines;
	var
		i: integer;
	begin
		i := VRemove(@myStackCheck)
	end;

{$ELSEC}

procedure ExitCoroutines;
	begin
	end;

{$ENDC}


end.