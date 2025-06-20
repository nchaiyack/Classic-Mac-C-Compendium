#   File:       Vectors.make
#   Target:     Vectors
#   Sources:    Acurs.c CVectors.c
#   Created:	Wednesday, May 5, 1993 9:29:57 PM

RECON = ::Recon:
SymOptions = -sym on
FlagOptions = -d CMLmake=1 -d VECTORS=1
COptions = -r {SymOptions} {FlagOptions}

OBJECTS = �
	:cvobj:CVRoot.c.o �
	:cvobj:CVectors.c.o �
	:cvobj:CVDocs.c.o �
	:cvobj:CVRouts.c.o �
	:cvobj:CVTextDoc.c.o �
	:cvobj:CVEntry.c.o �
	:obj:TE32K.c.o �
	:obj:Acurs.c.o �
	:cvobj:CMLmain.c.o �
	:cvobj:CMLmem.c.o �
	:cvobj:CMLdirs.c.o �
	:cvobj:CMLrouts.c.o �
	:cvobj:CMLrep.c.o �
	:cvobj:CMLp1.c.o �
	:cvobj:CMLp2.c.o �
	:cvobj:CMLclose.c.o

Vectors �� Vectors.make CVectors.r
	Rez VTypes.r �
		"{RIncludes}"Types.r �
		"{RIncludes}"SysTypes.r �
		CVectors.r �
		-append -o Vectors
		
Vectors �� Vectors.make CVHeaders {OBJECTS}
	Link -t APPL -c MRPG {SymOptions} -mf �
		{OBJECTS} �
		"{CLibraries}"CSANELib.o �
		"{CLibraries}"Math.o �
		#"{CLibraries}"Complex.o �
		"{CLibraries}"StdClib.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		-o Vectors

CVGlobs.h � Vectors.make CVRoot.c
	 MakeExtern CVRoot.c CVGlobs.h

CVHeaders � BuildCVHeaders.c CVGlobs.h CVectors.h
	C {COptions} BuildCVHeaders.c

CMLglobs.h � Vectors.make CMLroot.h
	 MakeExtern CMLroot.h CMLglobs.h

:cvobj:CVRoot.c.o � Vectors.make CVRoot.c
	 C {COptions} CVRoot.c -o :cvobj:CVRoot.c.o
:cvobj:CVectors.c.o � Vectors.make CVRoot.c CVectors.c
	 C -r {SymOptions} CVectors.c -o :cvobj:CVectors.c.o
:cvobj:CVDocs.c.o � Vectors.make CVGlobs.h CVDocs.c
	 C -r {SymOptions} CVDocs.c -o :cvobj:CVDocs.c.o
:cvobj:CVRouts.c.o � Vectors.make CVGlobs.h CVRouts.c
	 C -r {SymOptions} CVRouts.c -o :cvobj:CVRouts.c.o
:cvobj:CVTextDoc.c.o � Vectors.make CVGlobs.h CVTextDoc.c
	 C -r {SymOptions} CVTextDoc.c -o :cvobj:CVTextDoc.c.o
:cvobj:CVEntry.c.o � Vectors.make CVGlobs.h CVEntry.c
	 C -r {SymOptions} CVEntry.c -o :cvobj:CVEntry.c.o
:obj:Acurs.c.o � Vectors.make Acurs.c
	 C -r {SymOptions} -d NOMMAIN=1 Acurs.c -o :obj:Acurs.c.o
:obj:TE32K.c.o � Vectors.make TE32K.c
	 C -r {SymOptions} TE32K.c -o :obj:TE32K.c.o
:cvobj:CMLmain.c.o � Vectors.make CMLroot.h CMLmain.c
	 C -r {SymOptions} {COptions} CMLmain.c -o :cvobj:CMLmain.c.o
:cvobj:CMLmem.c.o � Vectors.make CMLglobs.h CML.h "{RECON}"MDLmem.c
	 C -o :cvobj:CMLmem.c.o {COptions} "{RECON}"MDLmem.c
:cvobj:CMLdirs.c.o � Vectors.make CMLglobs.h "{RECON}"MDLdirs.c
	 C -o :cvobj:CMLdirs.c.o {COptions} "{RECON}"MDLdirs.c
:cvobj:CMLrouts.c.o � Vectors.make CMLglobs.h CML.h "{RECON}"MDLrouts.c
	 C -o :cvobj:CMLrouts.c.o {COptions} "{RECON}"MDLrouts.c
:cvobj:CMLrep.c.o � Vectors.make CMLglobs.h "{RECON}"MDLrep.c
	 C -o :cvobj:CMLrep.c.o {COptions} "{RECON}"MDLrep.c
:cvobj:CMLp1.c.o � Vectors.make CMLglobs.h "{RECON}"MDLp1.c
	 C -o :cvobj:CMLp1.c.o {COptions} "{RECON}"MDLp1.c
:cvobj:CMLp2.c.o � Vectors.make CMLglobs.h "{RECON}"MDLp2.c
	 C -o :cvobj:CMLp2.c.o {COptions} "{RECON}"MDLp2.c
:cvobj:CMLclose.c.o � Vectors.make CMLglobs.h "{RECON}"MDLclose.c
	 C -o :cvobj:CMLclose.c.o {COptions} "{RECON}"MDLclose.c

