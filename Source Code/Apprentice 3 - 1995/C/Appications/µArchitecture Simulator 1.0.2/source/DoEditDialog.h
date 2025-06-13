enum basestrings {
kSTR_DECALLOWED = 128,
kSTR_HEXALLOWED,
kSTR_OCTALLOWED,
kSTR_BINALLOWED
};

Boolean DoEditDialog(short item, short class, short base);
Boolean DoEditDump(short *addr, short wordn);
