COptions=	-g

all=	abort.c.o �
	access.c.o �
	chdir.c.o �
	ctime.c.o �
	getbootvol.c.o �
	getwd.c.o �
	localtime.c.o �
	mkdir.c.o �
	opendir.c.o �
	perror.c.o �
	rename.c.o �
	rmdir.c.o �
	set_open_hook.c.o �
	stat.c.o �
	sync.c.o �
	time.c.o
	

lib.o � {all}
	lib -o lib.o -sn Main=unixemu {all}

libes=	{CLibraries}CInterface.o �
	{CLibraries}CRuntime.o �
	{CLibraries}CSANELib.o �
	{CLibraries}StdCLib.o

pobjs=	pwd.c.o lib.o
pwd	� {pobjs}
	link -ot MPST -oc 'MPS ' -p -o pwd {pobjs} {libes}

lobjs=	ls.c.o dir.c.o stat.c.o
ls	� {lobjs}
	link -ot MPST -oc 'MPS ' -o ls {lobjs} {libes}

mobjs=	mov.c.o rename.c.o perror.c.o
mov	� {mobjs}
	link -ot MPST -oc 'MPS ' -o mov {mobjs} {libes}

dobjs=	del.c.o rename.c.o perror.c.o
del	� {dobjs}
	link -ot MPST -oc 'MPS ' -o del {dobjs} {libes}
