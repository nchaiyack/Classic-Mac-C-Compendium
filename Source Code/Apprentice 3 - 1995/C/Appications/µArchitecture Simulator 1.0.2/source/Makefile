
Creator		= µSIM
Destf		= ArchSim.¹.rsrc

# versione USA
RezOptions		= -t rsrc -c RSED -i "{FabLibraries}" -s "{FabLibraries}" -append -d __USVersion__ -o {Targ}
Docs	= 'Simulator docs'
ExampleDoc	= 'Control Store Pad'
ExampleAsm	= 'Exercise.asm'
InstrDoc	= 'Mac-1 Instructions text'
ThngToDo	= 'To do in µSim'
VersFile	=	VersUSA.r
# versione Italiana
#RezOptions		= -append -d __VersioneItaliana__ -o {Targ}
#Docs	= 'Documentazione Disk Charmer'
#VersFile	=	VersItalia.r


AsmOptions		= -t -o "{ObjDir}"

#### Compiler & linker variable options

#MacsbugOptions	=	
#SymOptions	=	-sym on
#CSymOptions	=	{MacsbugOptions} {SymOptions} -y {FabDumpsSym} -k {FabDumpsSym}

#MacsbugOptions	=	
#SymOptions	=	
#CSymOptions	=	{MacsbugOptions} {SymOptions} -y {FabDumps} -k {FabDumps}

MacsbugOptions	=	-mbg off
SymOptions	=	
CSymOptions	=	{MacsbugOptions} {SymOptions} -y {FabDumps} -k {FabDumps}


# C & Link options
COptions	= {CSymOptions} -i "{FabLibraries}" -t -o "{ObjDir}" -d MPW3 -r -d SystemSevenOrLater -b2 -opt full
LinkOptions	= -rn -c {Creator} -d -mf {SymOptions} -l > Sim.map
	

OBJECTS = ¶
		"{FabLibraries}"FabLibs.o		¶
		"{ObjDir}"SimAsm.a.o			¶
		"{ObjDir}"AEHandlers.c.o		¶
		"{ObjDir}"Animation.c.o			¶
		"{ObjDir}"Assembler.c.o			¶
		"{ObjDir}"ControlStore.c.o		¶
		"{ObjDir}"Disasm.c.o			¶
		"{ObjDir}"DoEditDialog.c.o		¶
		"{ObjDir}"DoMenu.c.o			¶
		"{ObjDir}"Dump.c.o				¶
		"{ObjDir}"Globals.c.o			¶
		"{ObjDir}"InitMenus.c.o			¶
		"{ObjDir}"Input.c.o				¶
		"{ObjDir}"Main.c.o				¶
		"{ObjDir}"Microprogram_Ed.c.o	¶
		"{ObjDir}"myMemory.c.o			¶
		"{ObjDir}"Preferences.c.o		¶
		"{ObjDir}"Registers.c.o			¶
		"{ObjDir}"Scroll.c.o			¶
		"{ObjDir}"Simulator.c.o			¶
		"{ObjDir}"SimUtils.c.o			¶
		"{ObjDir}"TrackThumb.c.o
		

{Destf} ÄÄ {VersFile} {ThngToDo}
	Rez {VersFile} -append -o {ThngToDo}
{Destf} ÄÄ {VersFile} {InstrDoc}
	Rez {VersFile} -append -o {InstrDoc}
{Destf} ÄÄ {VersFile} {ExampleDoc}
	Rez {VersFile} -append -o {ExampleDoc}
{Destf} ÄÄ {VersFile} {ExampleAsm}
	Rez {VersFile} -append -o {ExampleAsm}
{Destf} ÄÄ {VersFile} {Docs}
	Rez {VersFile} -append -o {Docs}
{Destf} ÄÄ C.r "{FabLibraries}"GeneralUsageItalia.r "{FabLibraries}"GeneralUsageUSA.r USAC.r VersUSA.r
	Rez C.r {RezOptions}
	SetFile -a BM {Targ}
{Destf} ÄÄ CommonC.r BalloonC.r
	Rez CommonC.r {RezOptions}
{Destf} ÄÄ Sim.rsrc ReseditC.r
	Rez ReseditC.r {RezOptions}

#Sim ÄÄ {OBJECTS} Makefile
#	Link {LinkOptions} ¶
#		-ra Main=preload,locked,protected ¶
#		-ra Microprog=preload,locked,protected ¶
#		-ra STDCLIB=preload,locked,protected ¶
#		-ra Rare=purgeable,protected ¶
#		-ra About=protected,purgeable ¶
#		-ra Init=preload,protected,purgeable ¶
#		-ra %A5Init=preload,protected,purgeable ¶
#		-ra CleanUp=protected,purgeable ¶
#		{OBJECTS} ¶
#		"{Libraries}"Runtime.o ¶
#		"{Libraries}"Interface.o ¶
#		"{CLibraries}"StdClib.o ¶
#		-o {Targ}

#"{ObjDir}"SimAsm.a.o ÄÄ SimAsm.a
#	Asm {AsmOptions} SimAsm.a
#
#"{ObjDir}"AEHandlers.c.o Ä AEHandlers.c AEHandlers.h Assembler.h myMemory.h Main.h ControlStore.h DoMenu.h Globals.h SimUtils.h
#	 {C} {COptions} AEHandlers.c
#"{ObjDir}"Animation.c.o Ä Animation.c Animation.h DoMenu.h Main.h ControlStore.h Globals.h DoEditDialog.h Registers.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Animation.c
#"{ObjDir}"Assembler.c.o Ä Assembler.c Assembler.h Main.h Dump.h Disasm.h SimUtils.h Globals.h DoEditDialog.h Microprogram_Ed.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Assembler.c
#"{ObjDir}"ControlStore.c.o Ä ControlStore.c ControlStore.h DoMenu.h Animation.h SimUtils.h Globals.h Microprogram_Ed.h "{FabLibraries}"Utils.h
#	 {C} {COptions} ControlStore.c
#"{ObjDir}"Disasm.c.o Ä Disasm.c Disasm.h SimAsm.h TrackThumb.h Assembler.h Globals.h Main.h Microprogram_Ed.h Input.h Registers.h "{FabLibraries}"Utils.h 
#	 {C} {COptions} Disasm.c
#"{ObjDir}"DoEditDialog.c.o Ä DoEditDialog.c DoEditDialog.h Main.h SimUtils.h Animation.h Globals.h Registers.h "{FabLibraries}"Utils.h
#	 {C} {COptions} DoEditDialog.c
#"{ObjDir}"DoMenu.c.o Ä DoMenu.c DoMenu.h Animation.h Disasm.h Globals.h Input.h myMemory.h Simulator.h Registers.h Assembler.h Dump.h Main.h ControlStore.h Preferences.h SimUtils.h Microprogram_Ed.h "{FabLibraries}"Utils.h
#	 {C} {COptions} DoMenu.c
#"{ObjDir}"Dump.c.o Ä Dump.c Dump.h "{FabLibraries}"CursorBalloon.h DoMenu.h DoEditDialog.h TrackThumb.h Scroll.h Main.h Disasm.h Globals.h Input.h Registers.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Dump.c
#"{ObjDir}"Globals.c.o Ä Globals.c Globals.h
#	 {C} {COptions} Globals.c
#"{ObjDir}"InitMenus.c.o Ä InitMenus.c InitMenus.h SimAsm.h "{FabLibraries}"InitCursorBalloon.h "{FabLibraries}"CursorBalloon.h DoMenu.h Main.h ControlStore.h AEHandlers.h Animation.h Disasm.h DoEditDialog.h ¶
#		Globals.h Input.h Microprogram_Ed.h Registers.h SimUtils.h "{FabLibraries}"Utils.h
#	 {C} {COptions} InitMenus.c
#"{ObjDir}"Input.c.o Ä Input.c Input.h "{FabLibraries}"CursorBalloon.h Globals.h Main.h SimUtils.h
#	 {C} {COptions} Input.c
#"{ObjDir}"Main.c.o Ä Main.c Main.h InitMenus.h Simulator.h Input.h Preferences.h AEHandlers.h ControlStore.h DoMenu.h Disasm.h Dump.h "{FabLibraries}"CursorBalloon.h Globals.h Microprogram_Ed.h SimUtils.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Main.c
#"{ObjDir}"Microprogram_Ed.c.o Ä Microprogram_Ed.c Microprogram_Ed.h DoMenu.h ControlStore.h Main.h SimAsm.h Animation.h Globals.h "{FabLibraries}"CursorBalloon.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Microprogram_Ed.c
#"{ObjDir}"myMemory.c.o Ä myMemory.c myMemory.h Animation.h Disasm.h Dump.h Registers.h Globals.h SimUtils.h "{FabLibraries}"Utils.h
#	 {C} {COptions} myMemory.c
#"{ObjDir}"Preferences.c.o Ä Preferences.c Preferences.h Main.h Disasm.h DoEditDialog.h Dump.h Globals.h Registers.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Preferences.c
#"{ObjDir}"Registers.c.o Ä Registers.c Registers.h DoEditDialog.h Disasm.h Main.h Globals.h SimUtils.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Registers.c
#"{ObjDir}"Scroll.c.o Ä Scroll.c Scroll.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Scroll.c
#"{ObjDir}"Simulator.c.o Ä Simulator.c Simulator.h SimUtils.h Input.h AEHandlers.h Animation.h Dump.h Globals.h Microprogram_Ed.h Registers.h "{FabLibraries}"Utils.h
#	 {C} {COptions} Simulator.c
#"{ObjDir}"SimUtils.c.o Ä SimUtils.c SimUtils.h Main.h Preferences.h Animation.h ControlStore.h Disasm.h Dump.h Globals.h Input.h Registers.h "{FabLibraries}"Utils.h
#	 {C} {COptions} SimUtils.c
#"{ObjDir}"TrackThumb.c.o Ä TrackThumb.c TrackThumb.h
#	 {C} {COptions} TrackThumb.c
