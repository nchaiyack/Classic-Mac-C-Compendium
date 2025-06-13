CC=C
COptions= -D RTT -b2 -mbg off -r -d MPW -d MPWFncs -d MacToolboxFncs -D TokDotH="¶"::rtt:ltoken.h¶""
LinkOpts= -d -c 'MPS ' -t MPST
YFLAGS=
MAKE = make

ROBJS = rttparse.c.o rttmain.c.o rttlex.c.o rttsym.c.o rttnode.c.o rttout.c.o rttmisc.c.o ¶
	  rttdb.c.o rttinlin.c.o rttilc.c.o

PP_DIR = ::preproc:
P_DOT_H = {PP_DIR}preproc.h {PP_DIR}pproto.h ltoken.h ::h:mproto.h ¶
        ::h:define.h ::h:config.h ::h:typedefs.h ::h:proto.h ¶
        ::h:cstructs.h ::h:cpuconf.h
POBJS = pout.c.o pchars.c.o  perr.c.o pmem.c.o  bldtok.c.o macro.c.o preproc.c.o ¶
	evaluate.c.o files.c.o gettok.c.o pinit.c.o

COBJS = ::common:getopt.c.o ::common:time.c.o ::common:filepart.c.o ¶
	  ::common:memory.c.o ::common:identify.c.o ::common:strtbl.c.o ¶
	  ::common:rtdb.c.o ::common:mlocal.c.o ::common:literals.c.o  ¶
	  ::common:alloc.c.o

ICOBJS=	getopt.c.o time.c.o filepart.c.o memory.c.o identify.c.o strtbl.c.o rtdb.c.o ¶
	  mlocal.c.o literals.c.o alloc.c.o

OBJ = {ROBJS} {POBJS} {COBJS}

.c.o Ä .c
	{C} {DepDir}{Default}.c -o {TargDir}{Default}.c.o -s {Default} {COptions}

allÄ
	cd ::common; {MAKE} {ICOBJS}
	{MAKE} rtt

rttÄ	{OBJ}
	Link {LinkOpts} ¶
		-sg Seg_evaluate=rttmisc,pmem,pout,rttnode,gettok,pinit,rttsym,evaluate ¶
		-sg Seg_rttdb=pchars,rttmain,preproc,bldtok,rttilc,rttdb ¶
		-sg Seg_rttout=perr,rttlex,rttout ¶
		-sg Seg_rttparse=files,macro,rttinlin,rttparse ¶
		{OBJ} ¶
		#"{CLibraries}"CSANELib.o ¶
		#"{CLibraries}"Math.o ¶
		#"{CLibraries}"Complex.o ¶
		"{CLibraries}"StdClib.o ¶
		"{Libraries}"Stubs.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		#"{Libraries}"ToolLibs.o ¶
		-o rtt
	cp rtt :::bin

libraryÄ	{OBJ}
		rm -rf rtt.a
		ar qc rtt.a {OBJ}
		-(test -f :::NoRanlib) || (ranlib rtt.a)

{ROBJS}Ä rtt.h rtt1.h rttproto.h {P_DOT_H}

rttdb.c.oÄ ::h:version.h
rttparse.c.o Ä ::h:gsupport.h ::h:path.h ::h:config.h ::h:cstructs.h  ¶
	::h:proto.h ::h:typedefs.h ::h:cpuconf.h ::h:define.h
	{C} rttparse.c -o rttparse.c.o -s rttparse {COptions} -m
rttmain.c.o Ä ::h:path.h

pout.c.oÄ {PP_DIR}pout.c {P_DOT_H}
	{CC} -o pout.c.o {COptions} {PP_DIR}pout.c 

pchars.c.oÄ {PP_DIR}pchars.c {P_DOT_H}
	{CC} -o pchars.c.o {COptions} {PP_DIR}pchars.c 

perr.c.oÄ {PP_DIR}perr.c {P_DOT_H}
	{CC} -o perr.c.o {COptions} {PP_DIR}perr.c

pmem.c.oÄ {PP_DIR}pmem.c {P_DOT_H}
	{CC} -o pmem.c.o {COptions} {PP_DIR}pmem.c

bldtok.c.oÄ {PP_DIR}bldtok.c {P_DOT_H}
	{CC} -o bldtok.c.o {COptions} {PP_DIR}bldtok.c

macro.c.oÄ {PP_DIR}macro.c {P_DOT_H}
	{CC} -o macro.c.o {COptions} {PP_DIR}macro.c

preproc.c.oÄ {PP_DIR}preproc.c {P_DOT_H}
	{CC} -o preproc.c.o {COptions} {PP_DIR}preproc.c

evaluate.c.oÄ {PP_DIR}evaluate.c {P_DOT_H}
	{CC} -o evaluate.c.o {COptions} {PP_DIR}evaluate.c

files.c.oÄ {PP_DIR}files.c {P_DOT_H}
	{CC} -o files.c.o {COptions} {PP_DIR}files.c

gettok.c.oÄ {PP_DIR}gettok.c {P_DOT_H}
	{CC} -o gettok.c.o {COptions} {PP_DIR}gettok.c

pinit.c.oÄ {PP_DIR}pinit.c {P_DOT_H}
	{CC} -o pinit.c.o {COptions} {PP_DIR}pinit.c

#
# The following entry is commented out because it is not normally
# necessary to recreate rttparse.c and ltoken.h unless the grammar
# in rttgram.y for the run-time langauge is changed. Recreating these
# files is not normally a part of the installation process. Note that
# on some systems, yacc may not have large enough internal tables to
# translate this grammar.
#
#rttparse.c ltoken.hÄ rttgram.y
#	yacc -d {YFLAGS} rttgram.y
#	fgrep -v -x "extern char Åmalloc(), Årealloc();" y.tab.c > rttparse.c
#	rm y.tab.c
#	mv y.tab.h ltoken.h
