enum instrClass {
kCLASS_4_12 = 0,
kCLASS_5_11,
kCLASS_8_8,
kCLASS_16_0,
kCLASS_16_16,
kCLASS_16_16_REL,
kCLASS_4_12_REL
};

enum asmErrs {
kasmErrInvalidOpcode = 1,
kasmErrOpcodeNotExistent,
kasmErrPotHeapDamage,
kasmErrSymbolNotDef
};

enum asmAlrts {
kALRT_ASM = 511,
kALRT_INVALIDOPC,
kALRT_OPCNOTEXIS,
kALRT_HEAPCHECK,
kALRT_SYMNOTDEF
};

OSErr myAsmFile(FSSpec *theFile);
void DoAsmPrefsDialog(void);
