CC=C
COptions= -b2 -mbg off -r -d MPW -d MPWFncs -d MacToolboxFncs -opt full
LinkOpts= -d -c 'MPS ' -t MPST
MAKE=make

TRANS=		trans.c.o tcode.c.o tlex.c.o lnklist.c.o tparse.c.o tsym.c.o tmem.c.o tree.c.o

LINKR=		link.c.o lglob.c.o lcode.c.o llex.c.o lmem.c.o lsym.c.o opcode.c.o

OBJS=		tmain.c.o util.c.o tlocal.c.o {TRANS} {LINKR}

COBJS=		::common:long.c.o ::common:getopt.c.o ::common:alloc.c.o  �
		   ::common:filepart.c.o ::common:strtbl.c.o ::common:ipp.c.o

ICOBJS=		long.c.o getopt.c.o alloc.c.o filepart.c.o strtbl.c.o ipp.c.o

PGMS=		icont

.c.o � .c
	{C} {DepDir}{Default}.c -o {TargDir}{Default}.c.o -s {Default} {COptions}

all�		{PGMS}

common�
		cd ::common; {MAKE} {ICOBJS}

icont�		hdr.h common {OBJS} {COBJS}
	Link {LinkOpts} �
		-sg Seg_lsym=lnklist,util,tmem,tlocal,trans,tree,lglob,link,lmem,lsym �
		-sg Seg_tcode=opcode,tmain,tsym,llex,lcode,tparse,tlex,tcode �
		{OBJS} {COBJS} �
		#"{CLibraries}"CSANELib.o �
		#"{CLibraries}"Math.o �
		#"{CLibraries}"Complex.o �
		"{CLibraries}"StdClib.o �
		"{Libraries}"Stubs.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		"{Libraries}"ToolLibs.o �
		-o icont
	rez -a -m -o icont -ov icont.r
	cp icont :::bin

{OBJS}�   ::h:define.h ::h:config.h ::h:cpuconf.h ::h:gsupport.h  �
		   ::h:proto.h ::h:mproto.h ::h:dproto.h  �
		   ::h:typedefs.h ::h:cstructs.h tproto.h

{COBJS}�	::h:mproto.h

tmain.c.o�	tglobals.h ::h:path.h
util.c.o�		tglobals.h tree.h ::h:fdefs.h

# translator files
trans.c.o�	tglobals.h tsym.h ttoken.h tree.h ::h:version.h ::h:kdefs.h
lnklist.c.o�	lfile.h
tparse.c.o�	::h:lexdef.h tglobals.h tsym.h tree.h keyword.h
tcode.c.o�	tglobals.h tsym.h ttoken.h tree.h
tlex.c.o�		::h:lexdef.h ::h:parserr.h ttoken.h tree.h ::h:esctab.h  �
		   ::common:lextab.h ::common:yylex.h ::common:error.h
tmem.c.o�		tglobals.h tsym.h tree.h
tree.c.o�		tree.h
tsym.c.o�		tglobals.h tsym.h ttoken.h lfile.h keyword.h ::h:kdefs.h

# linker files
{LINKR}�	link.h lfile.h ::h:rt.h ::h:sys.h ::h:monitor.h  �
		   ::h:rstructs.h ::h:rmacros.h ::h:rexterns.h

link.c.o�		tglobals.h ::h:header.h hdr.h
lcode.c.o�	tglobals.h opcode.h keyword.h ::h:header.h  �
			::h:opdefs.h ::h:version.h
lglob.c.o�	opcode.h ::h:opdefs.h ::h:version.h
llex.c.o�		tglobals.h opcode.h ::h:opdefs.h
lmem.c.o�		tglobals.h
lsym.c.o�		tglobals.h
opcode.c.o�	opcode.h ::h:opdefs.h



#  The following sections are commented out because they do not need to be
#  performed unless changes are made to cgrammar.c, ::h:grammar.h,
#  ::common:tokens.txt, or ::common:op.txt.  Such changes involve
#  modifications to the syntax of Icon and are not part of the installation
#  process. However, if the distribution files are unloaded in a fashion
#  such that their dates are not set properly, the following sections would
#  be attempted.
#
#  Note that if any changes are made to the files mentioned above, the comment
#  characters at the beginning of the following lines should be removed.
#  icont must be on your search path for these actions to work.
#
#::common:lextab.h ::common:yacctok.h ::common:fixgram ::common:pscript�  �
#			::common:tokens.txt ::common:op.txt
#		cd ::common; make gfiles
#
#tparse.c ttoken.h�	tgram.g trash ::common:pscript
## expect 218 :shift:reduce conflicts
#		yacc -d tgram.g
#		:trash <y.tab.c | ::common:pscript >tparse.c
#		mv y.tab.h ttoken.h
#		rm -f y.tab.c
#
#tgram.g�	tgrammar.c ::h:define.h ::h:grammar.h  �
#			::common:yacctok.h ::common:fixgram 
#		{CC} -E -C tgrammar.c | ::common:fixgram >tgram.g
#
#::h:kdefs.h keyword.h�	::runtime:keyword.r mkkwd
#		:mkkwd <::runtime:keyword.r
#
#trash�		trash.icn
#		icont -s trash.icn
#
#mkkwd�		mkkwd.icn
#		icont -s mkkwd.icn
