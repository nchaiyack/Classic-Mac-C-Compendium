resobj = "{obj}"resolver:

OBJS = "{Libraries}"Interface.o �
	   "{CLibraries}"Math.o �
	   "{Libraries}"Runtime.o �
	   "{CLibraries}"StdCLib.o �
	   "{CLibraries}"CSANELib.o �
	   "{obj}"socket.o �
	   "{obj}"libresolv.o �
	   "{obj}"unixlib.c.o �
	   "{resobj}"dprintf.c.o

Coptions =  -s resolv -sym full -mbg full -i "{unixinc}" -i "{mactcp}" -i "{socket}" # -i "{app}" -DDEBUG -DXFER

MAP =  -uf uref -x xref -l > map
TOOL = -t MPST -c 'MPS '

all � {obj}libresolv.o nstest

clean �
	rm -i nstest

#########################################################################

RES_OBJS = "{resobj}"res_comp.c.o �
		   "{resobj}"res_debug.c.o �
		   "{resobj}"res_init.c.o �
		   "{resobj}"res_mkquery.c.o �
		   "{resobj}"res_query.c.o �
		   "{resobj}"res_send.c.o
		   
# herror.c.o gethostnamadr.c.o sethostent.c.o

{obj}libresolv.o � {RES_OBJS}
	lib -o {obj}libresolv.o {RES_OBJS}
	
"{resobj}"res_debug.c.o � res_debug.c
	C {COptions} -D DEBUG res_debug.c -o "{resobj}"res_debug.c.o

#########################################################################

NS_OBJS = nstest.c.o {OBJS}

nstest � {NS_OBJS}
	link {TOOL} {NS_OBJS} -o nstest

#########################################################################

"{resobj}" � "{resolver}"

.c.o	�	.c
	c "{depDir}{default}.c" -o "{targDir}{default}.c.o" {COptions}
