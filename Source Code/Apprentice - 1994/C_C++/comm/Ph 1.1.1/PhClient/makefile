Lib = "jlnx:Lib:"
MacTCPCIncludes = "jlnx:MacTCP:CIncludes:"
MacTCPLibraries = "jlnx:MacTCP:Libraries:"

Objects = ph.c.o glob.c.o edit.c.o serv.c.o mtcp.c.o �
	encrypt.c.o crypt.c.o abou.c.o tran.c.o login.c.o pswd.c.o open.c.o �
	new.c.o query.c.o help.c.o wstm.c.o site.c.o

COptions = -mbg on -sym off -r -i {MacTCPCIncludes} -i {Lib}
LDebugOptions = -sym off

ph.c.o � precompile {Lib}utl.h {Lib}oop.h {Lib}fsu.h �
	rez.h mtcp.h glob.h edit.h abou.h query.h help.h serv.h site.h
glob.c.o � precompile {Lib}utl.h {Lib}oop.h �
	serv.h rez.h mtcp.h glob.h rslv.h
edit.c.o � precompile {Lib}utl.h {Lib}oop.h �
	serv.h rez.h mtcp.h glob.h edit.h login.h pswd.h open.h new.h wstm.h
serv.c.o � precompile {Lib}utl.h �
	serv.h mtcp.h rez.h encrypt.h glob.h tran.h query.h help.h
mtcp.c.o � precompile {Lib}utl.h {Lib}oop.h �
	mtcp.h rslv.h serv.h glob.h
encrypt.c.o � crypt.h
abou.c.o � precompile {Lib}utl.h {Lib}oop.h �
	abou.h rez.h glob.h edit.h
tran.c.o � precompile {Lib}utl.h {Lib}oop.h �
	rez.h tran.h glob.h mtcp.h wstm.h
login.c.o � precompile {Lib}utl.h {Lib}oop.h �
	rez.h login.h glob.h wstm.h
pswd.c.o � precompile {Lib}utl.h {Lib}oop.h �
	rez.h pswd.h glob.h wstm.h
open.c.o � precompile {Lib}utl.h {Lib}oop.h �
	rez.h open.h glob.h wstm.h
new.c.o � precompile {Lib}utl.h {Lib}oop.h �
	rez.h new.h glob.h wstm.h
query.c.o � precompile {Lib}utl.h {Lib}oop.h �
	rez.h mtcp.h serv.h glob.h query.h wstm.h
help.c.o � precompile {Lib}oop.h �
	rez.h mtcp.h serv.h glob.h help.h wstm.h site.h
wstm.c.o � precompile {Lib}utl.h {Lib}oop.h  �
	glob.h wstm.h
rslv.c.o � precompile {Lib}utl.h {Lib}fsu.h �
	rslv.h
site.c.o � precompile {Lib}utl.h {Lib}oop.h �
	rez.h site.h glob.h wstm.h

precompile � precompile.c
	C {COptions} precompile.c

Ph �� rez.r
	Rez rez.r -append -o Ph
	
Ph �� balloons.r
	Rez balloons.r -append -o Ph
	
Ph �� {Objects} {Lib}Lib.o
	Link -t APPL -c 'PHED' -ra =$30 {LDebugOptions} �
		-sg Main=utl,STDCLIB �
		-sg init=fsui,rslvi �
		{Objects} �
		{Lib}Lib.o �
		"{Libraries}"Interface.o �
		"{Libraries}"Runtime.o �
		"{CLibraries}"StdCLib.o �
		"{MacTCPLibraries}"dnr.o �
		-o Ph
	setfile -a BM Ph
