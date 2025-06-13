/* PcodeDisassembly.h */

#ifndef Included_PcodeDisassembly_h
#define Included_PcodeDisassembly_h

/* PcodeDisassembly module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* PcodeObject */
/* Numbers */

/* forward declarations */
struct PcodeRec;

/* disassemble pcode and return a string block containing all the data */
char*							DisassemblePcode(struct PcodeRec* Pcode, char CarriageReturn);

#endif
