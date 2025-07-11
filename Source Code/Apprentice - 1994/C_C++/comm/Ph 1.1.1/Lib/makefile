#	makefile - Reusable code makefile.
#
#	I use my "Lib" folder to keep all of my reusable code. This code is
#	used, or could be used, by more than one program.
#
#	C modules.
#	----------
#
#  fsu - File system utilities.
#	hlp - Help window manager.
#	oop - Object-oriented window and command manager.
#	rep - Report manager.
#	rpp - Report printing manager.
#	scn - Volume scanning.
#	sfu - Standard file utilities.
#	tcg - Think C glue. (not used by MPW version).
#	utl - Utilities.
#	vol - Volume selection.
#
#	Assembler modules.
#	------------------
#
#	showinit - Mercer et al's ShowINIT.
#  SafeStart - Fran�ois Grieu's 64K ROM checker.
# 
#	Definition functions.
#	---------------------
#
#	ldf - Custom list definition function for reports.
#
#	Tools.
#	------
#
#	wrap - Wrap paragraphs.
#	cvrt - Compile document.

COptions = -mbg on -sym off -r

ObjFiles = 	�
			fsu.c.o �
			hlp.c.o �
			oop.c.o �
			rep.c.o �
			rpp.c.o �
			scn.c.o �
			sfu.c.o �
			utl.c.o �
			vol.c.o �

fsu.c.o � precompile utl.h fsu.h
hlp.c.o � precompile rep.h rpp.h utl.h hlp.h doc.h
oop.c.o � precompile oop.h utl.h
rep.c.o � precompile rep.h utl.h doc.h sfu.h fsu.h
rpp.c.o � precompile rpp.h utl.h doc.h
scn.c.o � precompile scn.h utl.h
sfu.c.o � precompile sfu.h utl.h
utl.c.o � precompile utl.h
vol.c.o � precompile vol.h utl.h

precompile � precompile.c
	C {COptions} precompile.c
	
#	Special "all" target to bring everything up-to-date.

all � lib.o showinit.o safestart.o ldf.o wrap cvrt
	
#	The library of reusable code modules.

lib.o � {ObjFiles}
	lib -o lib.o {ObjFiles}
	 
#	ShowINIT. 
	
showinit.o � showinit.a
	Asm -w showinit.a -o showinit.o
	
#	SafeStart

safestart.o � safestart.a
	Asm -w safestart.a -o safestart.o
	
#	List definition function.

ldf.o � ldf.c doc.h precompile
	C ldf.c -o ldf.o {COptions}
		
#	wrap tool.		
	
wrapObjects = wrap.c.o

wrap � {wrapObjects}
	Link -d -c 'MPS ' -t MPST �
		{wrapObjects} �
		"{Libraries}"stubs.o �
		"{Libraries}"Interface.o �
		"{Libraries}"Runtime.o �
		"{CLibraries}"StdCLib.o �
		-o wrap
		
#	cvrt tool.		
	
cvrtObjects = cvrt.c.o

cvrt � {cvrtObjects}
	Link -d -c 'MPS ' -t MPST �
		{cvrtObjects} �
		"{Libraries}"stubs.o �
		"{Libraries}"Interface.o �
		"{Libraries}"Runtime.o �
		"{CLibraries}"StdCLib.o �
		-o cvrt
