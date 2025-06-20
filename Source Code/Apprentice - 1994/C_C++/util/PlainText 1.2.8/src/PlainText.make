#   File:       PlainText.make
#   Target:     PlainText
#   Sources:    Acurs.c PlainText.c PTRoot.c PlainText.c PTDocs.c PTConverts.c
#   Created:	Monday, Nov 22, 1993

#SymOptions = -sym on
COptions = -r {SymOptions}

OBJECTS = �
	:obj:PTRoot.c.o �
	:obj:PlainText.c.o �
	:obj:PTDocs.c.o �
	:obj:PTConverts.c.o �
	:obj:TE32K.c.o �
	:obj:Acurs.c.o 


PlainText �� PlainText.make PlainText.r
	Rez "{RIncludes}"Types.r �
		"{RIncludes}"SysTypes.r �
		PlainText.r �
		-append -o PlainText

PlainText �� PlainText.make PTHeaders {OBJECTS}
	Link -t APPL -c MRPH {SymOptions} �
		{OBJECTS} �
		#"{CLibraries}"CSANELib.o �
		#"{CLibraries}"Math.o �
		#"{CLibraries}"Complex.o �
		"{CLibraries}"StdClib.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		-o PlainText
	SetFile -a B PlainText

PTGlobs.h � PlainText.make PTRoot.c
	 MakeExtern PTRoot.c PTGlobs.h
	 
PTHeaders � BuildPTHeaders.c PTGlobs.h PlainText.h
	C {COptions} BuildPTHeaders.c
	
:obj:PTRoot.c.o � PlainText.make PTRoot.c
	 C {COptions} PTRoot.c -o :obj:PTRoot.c.o
:obj:PlainText.c.o � PlainText.make PTRoot.c PlainText.c
	 C {COptions} PlainText.c -o :obj:PlainText.c.o
:obj:PTDocs.c.o � PlainText.make PTGlobs.h PTDocs.c
	 C {COptions} PTDocs.c -o :obj:PTDocs.c.o
:obj:PTConverts.c.o � PlainText.make PTGlobs.h PTConverts.c
	 C {COptions} PTConverts.c -o :obj:PTConverts.c.o
:obj:Acurs.c.o � PlainText.make Acurs.c
	 C {COptions} -d NOMMAIN=1 -o :obj:Acurs.c.o Acurs.c
:obj:TE32K.c.o � PlainText.make TE32K.c
	 C {COptions} -d NOMMAIN=1 -o :obj:TE32K.c.o TE32K.c
