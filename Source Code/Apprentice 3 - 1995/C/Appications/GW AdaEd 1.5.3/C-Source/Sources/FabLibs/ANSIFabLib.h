#ifndef _ANSIFabLib_h
#define _ANSIFabLib_h

struct fabFILE {
	unsigned char *Current;
	unsigned char *MyBuffer;
	Size	FileSize;		/* get the size of the source file */
	short	fileRefNum;
};

typedef struct fabFILE fabFILE;

fabFILE *fab_fopen(const char *sourcefilename, SInt8 permission);
void fab_fclose(fabFILE *theFile);
Boolean fab_feof(fabFILE *theFile);
int fab_getc(fabFILE *theFile);
Size fab_fread(char *dest, Size wanted, fabFILE *theFile);
Size fab_ftell(const fabFILE *file);
void fab_fseek(fabFILE *file, Size offset);

#endif

