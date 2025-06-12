// used by show init.c

struct QDGlobals {
	char privates[76];
	long randSeed;
	BitMap screenBits;
	Cursor arrow;
	Pattern dkGray;
	Pattern ltGray;
	Pattern gray;
	Pattern black;
	Pattern white;
	GrafPtr thePort;
	long	end;
};

typedef struct QDGlobals QDGlobals;
