struct Command {
	Byte	InstrumentNo;		// Instrument no
	Byte 	AmigaPeriod;		// Note, see table
	Byte 	EffectCmd;			// Effect cmd
	Byte 	EffectArg;			// Effect arg
};

struct Pattern {				// Pattern = 64 notes to play
	struct Command Commands[ 64];
};

struct FileInstrData {
	char 	Filename[ 32];		// Instrument's filename
	long 	insSize;			// Sample length
	Byte 	fineTune;
	Byte 	volume;			// Base volume
	long	freq;				// Base frequence, simple, double, quadruple
	Byte	amplitude;			// 8 or 16 bits
	long	loopStart;			// LoopStart
	long	loopLenght;			// LoopLength
};

struct SoundTrackSpec {
	long	MADIdentification;		// Mad Identification : 'MADF' 
	char 	NameSignature[ 32];	// Music's name
	struct 	FileInstrData fid[ 64];	// 64 instruments descriptor
	Byte	PatMax;
	Byte 	numPointers;			// Patterns number
	Byte	oPointers[ 128];		// Patterns list
	Byte	Tracks;					// Tracks number
};
typedef struct SoundTrackSpec SoundTrackSpec;