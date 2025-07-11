#ifndef THINK_C_5

/* to avoid the glue is faster and typically generates shorter code (the glue
itself is short but there are many of them, plus an entry in the jump table,
and the calling sequence, passing one extra parameter, is repeated a lot of times) */


/* from MPW: devices.h */

pascal OSErr PBControlSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA004,0x3E80};
    /* That is:
    MOVE.L  (SP)+,A0
	_Control
	MOVE.W	D0,(SP)
	i.e., the caller pushes the argument, the first inline word
	pops it, then the third inline word pushes the result code and
	the caller pops it. And this absurdity is still much more efficient
	than using the standard solution, the glue function PBControl.
	An improvement would be to declare all them as C functions, but
	then this header file could be nonportable among different compilers
	or different releases of the same compiler.
	MPW 3.2 solves the problem by a pragma which lets you declare that
	a function accepts arguments in registers and returns its value in a
	register, but that's a new feature. Think C 5 supports it !
	*/
pascal OSErr PBStatusSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA005,0x3E80};

/* from MPW: files.h */

pascal OSErr PBOpenSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA000,0x3E80};
pascal OSErr PBCloseSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA001,0x3E80};
pascal OSErr PBReadSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA002,0x3E80};
pascal OSErr PBWriteSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA003,0x3E80};
pascal OSErr PBGetVInfoSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA007,0x3E80};
pascal OSErr PBGetVolSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA014,0x3E80};
pascal OSErr PBSetVolSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA015,0x3E80};
pascal OSErr PBFlushVolSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA013,0x3E80};
pascal OSErr PBCreateSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA008,0x3E80};
pascal OSErr PBDeleteSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA009,0x3E80};
pascal OSErr PBOpenDFSync(ParmBlkPtr paramBlock)
    = {0x205F,0x701A,0xA060,0x3E80};
pascal OSErr PBOpenRFSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA00A,0x3E80};
pascal OSErr PBGetFInfoSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA00C,0x3E80};
pascal OSErr PBSetFInfoSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA00D,0x3E80};
pascal OSErr PBAllocateSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA010,0x3E80};
pascal OSErr PBGetEOFSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA011,0x3E80};
pascal OSErr PBSetEOFSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA012,0x3E80};
pascal OSErr PBFlushFileSync(ParmBlkPtr paramBlock)
    = {0x205F,0xA045,0x3E80};
pascal OSErr PBOpenWDSync(WDPBPtr paramBlock)
    = {0x205F,0x7001,0xA260,0x3E80};
pascal OSErr PBCloseWDSync(WDPBPtr paramBlock)
    = {0x205F,0x7002,0xA260,0x3E80};
pascal OSErr PBHSetVolSync(WDPBPtr paramBlock)
    = {0x205F,0xA215,0x3E80};
pascal OSErr PBHGetVolSync(WDPBPtr paramBlock)
    = {0x205F,0xA214,0x3E80};
pascal OSErr PBDirCreateSync(HParmBlkPtr paramBlock)
    = {0x205F,0x7006,0xA260,0x3E80};
pascal OSErr PBGetWDInfoSync(WDPBPtr paramBlock)
    = {0x205F,0x7007,0xA260,0x3E80};
pascal OSErr PBGetFCBInfoSync(FCBPBPtr paramBlock)
    = {0x205F,0x7008,0xA260,0x3E80};
pascal OSErr PBGetCatInfoSync(CInfoPBPtr paramBlock)
    = {0x205F,0x7009,0xA260,0x3E80};
pascal OSErr PBSetCatInfoSync(CInfoPBPtr paramBlock)
    = {0x205F,0x700A,0xA260,0x3E80};
pascal OSErr PBHGetVInfoSync(HParmBlkPtr paramBlock)
    = {0x205F,0xA207,0x3E80};
pascal OSErr PBHOpenSync(HParmBlkPtr paramBlock)
    = {0x205F,0xA200,0x3E80};
pascal OSErr PBHOpenRFSync(HParmBlkPtr paramBlock)
    = {0x205F,0xA20A,0x3E80};
pascal OSErr PBHOpenDFSync(HParmBlkPtr paramBlock)
    = {0x205F,0x701A,0xA260,0x3E80};
pascal OSErr PBHCreateSync(HParmBlkPtr paramBlock)
    = {0x205F,0xA208,0x3E80};
pascal OSErr PBHDeleteSync(HParmBlkPtr paramBlock)
    = {0x205F,0xA209,0x3E80};
pascal OSErr PBHGetFInfoSync(HParmBlkPtr paramBlock)
    = {0x205F,0xA20C,0x3E80};
pascal OSErr PBHSetFInfoSync(HParmBlkPtr paramBlock)
    = {0x205F,0xA20D,0x3E80};

#endif
