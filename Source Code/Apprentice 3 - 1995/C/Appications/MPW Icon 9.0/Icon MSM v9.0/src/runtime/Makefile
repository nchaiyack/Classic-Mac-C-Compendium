COptions= -b2 -mbg off -r -d MPW -d MACINTOSH -d MPWFncs -d MacToolboxFncs -opt full
LinkOpts= -d -c 'MPS ' -t MPST
CC=C
RTT=:::bin:rtt
MAKE=make

HDRS = ::h:define.h ::h:config.h ::h:typedefs.h ::h:monitor.h �
	  ::h:proto.h ::h:cstructs.h ::h:cpuconf.h ::h:grttin.h �
	  ::h:rmacros.h ::h:rexterns.h ::h:rstructs.h  �
	  ::h:rproto.h ::h:mproto.h ::h:version.h ::h:sys.h

XICONHDRS = 
# ::h:xicon.h ::h:xwin.h

all�	interp_all

.c.o � .c
	{C} {DepDir}{Default}.c -o {TargDir}{Default}.c.o -s {Default} {COptions} {SymOption}

####################################################################
#
# Make entries for iconx
#

XOBJS=	xcnv.c.o xdata.c.o xdef.c.o xerrmsg.c.o xextcall.c.o xfconv.c.o xfmath.c.o �
	xfmisc.c.o xfmonitr.c.o xfscan.c.o xfstr.c.o xfstranl.c.o xfstruct.c.o xfsys.c.o �
	xfwindow.c.o ximain.c.o ximisc.c.o xinit.c.o xinterp.c.o xinvoke.c.o �
	xistart.c.o xkeyword.c.o xlmisc.c.o xoarith.c.o xoasgn.c.o xocat.c.o xocomp.c.o �
	xomisc.c.o xoref.c.o xoset.c.o xovalue.c.o xralc.c.o xrcoexpr.c.o xrcomp.c.o �
	xrdebug.c.o xrlocal.c.o xrlrgint.c.o xrmemmgt.c.o xrmisc.c.o xrstruct.c.o xrsys.c.o �
	xrwinrsc.c.o xrgfxsys.c.o xrwinsys.c.o xrwindow.c.o xfxtra.c.o

COBJS=	::common:long.c.o ::common:memory.c.o ::common:time.c.o ::common:save.c.o  �
	::common:rswitch.a.o ::common:redirerr.c.o ::common:xwindow.c.o

ICOBJS=	long.c.o memory.c.o time.c.o save.c.o rswitch.a.o redirerr.c.o xwindow.c.o

OBJS=	{XOBJS} {COBJS}

interp_all�
	cd ::common; {MAKE} {ICOBJS}
	{MAKE} iconx

iconx� {OBJS}
	Link {LinkOpts} �
		-sg Seg_xfmisc=xfstruct,xcnv,xoref,xrdebug,xfmisc �
		-sg Seg_xoarith=xrsys,xerrmsg,xdef,xfconv,ximisc,xinvoke,xomisc,xoset,xrstruct,xfmath,xinit,xfstranl,xocomp,xkeyword,xoarith �
		-sg Seg_xoasgn=ximain,xfstr,xfsys,xrmemmgt,xralc,xinterp,xoasgn �
		-sg Seg_xocat=xovalue,xrlocal,xlmisc,xfscan,xrcoexpr,xocat �
		-sg Seg_xrlrgint=xfmonitr,xrwindow,xdata,xfxtra,xrwinsys,xfwindow,xistart,xrgfxsys,xrwinrsc,xextcall,xrcomp,xrmisc,xrlrgint �
		{OBJS} �
		"{CLibraries}"CSANELib.o �
		"{CLibraries}"Math.o �
		#"{CLibraries}"Complex.o �
		"{CLibraries}"StdClib.o �
		"{Libraries}"Stubs.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		"{Libraries}"ToolLibs.o �
		-o iconx
		cp iconx :::bin

xcnv.c.o� cnv.r {HDRS}
	:::bin:rtt -x cnv.r
	{CC} -s cnv {COptions} xcnv.c
	rm xcnv.c

xdata.c.o� data.r {HDRS} ::h:kdefs.h ::h:fdefs.h ::h:odefs.h
	:::bin:rtt -x data.r
	{CC} -s data {COptions} xdata.c
	rm xdata.c

xdef.c.o� def.r {HDRS}
	:::bin:rtt -x def.r
	{CC} -s def {COptions} xdef.c
	rm xdef.c

xerrmsg.c.o� errmsg.r {HDRS}
	:::bin:rtt -x errmsg.r
	{CC} -s errmsg {COptions} xerrmsg.c
	rm xerrmsg.c

xextcall.c.o� extcall.r {HDRS}
	:::bin:rtt -x extcall.r
	{CC} -s extcall {COptions} xextcall.c
	rm xextcall.c

xfconv.c.o� fconv.r {HDRS}
	:::bin:rtt -x fconv.r
	{CC} -s fconv {COptions} xfconv.c
	rm xfconv.c

xfmath.c.o� fmath.r {HDRS}
	:::bin:rtt -x fmath.r
	{CC} -s fmath {COptions} xfmath.c
	rm xfmath.c

xfmisc.c.o� fmisc.r {HDRS}
	:::bin:rtt -x fmisc.r
	{CC} -s fmisc {COptions} xfmisc.c
	rm xfmisc.c

xfmonitr.c.o� fmonitr.r {HDRS}
	:::bin:rtt -x fmonitr.r
	{CC} -s  fmonitr{COptions} xfmonitr.c
	rm xfmonitr.c

xfscan.c.o� fscan.r {HDRS}
	:::bin:rtt -x fscan.r
	{CC} -s fscan {COptions} xfscan.c
	rm xfscan.c

xfstr.c.o� fstr.r {HDRS}
	:::bin:rtt -x fstr.r
	{CC} -s fstr {COptions} xfstr.c
	rm xfstr.c

xfstranl.c.o� fstranl.r {HDRS}
	:::bin:rtt -x fstranl.r
	{CC} -s fstranl {COptions} xfstranl.c
	rm xfstranl.c

xfstruct.c.o� fstruct.r {HDRS}
	:::bin:rtt -x fstruct.r
	{CC} -s fstruct {COptions} xfstruct.c
	rm xfstruct.c

xfsys.c.o� fsys.r {HDRS}
	:::bin:rtt -x fsys.r
	{CC} -s fsys {COptions} xfsys.c
	rm xfsys.c

xfwindow.c.o� fwindow.r {HDRS} {XICONHDRS}
	:::bin:rtt -x fwindow.r
	{CC} -s fwindow {COptions} xfwindow.c
	rm xfwindow.c

ximain.c.o� imain.r {HDRS}
	:::bin:rtt -x imain.r
	{CC} -s imain {COptions} ximain.c
	rm ximain.c

ximisc.c.o� imisc.r {HDRS}
	:::bin:rtt -x imisc.r
	{CC} -s imisc {COptions} ximisc.c
	rm ximisc.c

xinit.c.o� init.r {HDRS}
	:::bin:rtt -x init.r
	{CC} -s init {COptions} xinit.c
	rm xinit.c

xinterp.c.o� interp.r {HDRS}
	:::bin:rtt -x interp.r
	{CC} -s interp {COptions} xinterp.c
	rm xinterp.c

xinvoke.c.o� invoke.r {HDRS}
	:::bin:rtt -x invoke.r
	{CC} -s invoke {COptions} xinvoke.c
	rm xinvoke.c

xistart.c.o� istart.r {HDRS}
	:::bin:rtt -x istart.r
	{CC} -s istart {COptions} xistart.c
	rm xistart.c

xkeyword.c.o� keyword.r {HDRS} ::h:features.h
	:::bin:rtt -x keyword.r
	{CC} -s keyword {COptions} xkeyword.c
	rm xkeyword.c

xlmisc.c.o� lmisc.r {HDRS}
	:::bin:rtt -x lmisc.r
	{CC} -s lmisc {COptions} xlmisc.c
	rm xlmisc.c

xoarith.c.o� oarith.r {HDRS}
	:::bin:rtt -x oarith.r
	{CC} -s oarith {COptions} xoarith.c
	rm xoarith.c

xoasgn.c.o� oasgn.r {HDRS}
	:::bin:rtt -x oasgn.r
	{CC} -s oasgn {COptions} xoasgn.c
	rm xoasgn.c

xocat.c.o� ocat.r {HDRS}
	:::bin:rtt -x ocat.r
	{CC} -s ocat {COptions} xocat.c
	rm xocat.c

xocomp.c.o� ocomp.r {HDRS}
	:::bin:rtt -x ocomp.r
	{CC} -s ocomp {COptions} xocomp.c
	rm xocomp.c

xomisc.c.o� omisc.r {HDRS}
	:::bin:rtt -x omisc.r
	{CC} -s omisc {COptions} xomisc.c
	rm xomisc.c

xoref.c.o� oref.r {HDRS}
	:::bin:rtt -x oref.r
	{CC} -s oref {COptions} xoref.c
	rm xoref.c

xoset.c.o� oset.r {HDRS}
	:::bin:rtt -x oset.r
	{CC} -s oset {COptions} xoset.c
	rm xoset.c

xovalue.c.o� ovalue.r {HDRS}
	:::bin:rtt -x ovalue.r
	{CC} -s ovalue {COptions} xovalue.c
	rm xovalue.c

xralc.c.o� ralc.r {HDRS}
	:::bin:rtt -x ralc.r
	{CC} -s ralc {COptions} xralc.c
	rm xralc.c

xrcoexpr.c.o� rcoexpr.r {HDRS}
	:::bin:rtt -x rcoexpr.r
	{CC} -s rcoexpr {COptions} xrcoexpr.c
	rm xrcoexpr.c

xrcomp.c.o� rcomp.r {HDRS}
	:::bin:rtt -x rcomp.r
	{CC} -s rcomp {COptions} xrcomp.c
	rm xrcomp.c

xrdebug.c.o� rdebug.r {HDRS}
	:::bin:rtt -x rdebug.r
	{CC} -s rdebug {COptions} xrdebug.c
	rm xrdebug.c

xrlocal.c.o� rlocal.r {HDRS}
	cp rlocal.r xrlocal.c
	{CC} -s xrlocal {COptions} xrlocal.c
	rm xrlocal.c

xrlrgint.c.o� rlrgint.r {HDRS}
	:::bin:rtt -x rlrgint.r
	{CC} -s rlrgint {COptions} xrlrgint.c
	rm xrlrgint.c

xrmemmgt.c.o� rmemmgt.r {HDRS}
	:::bin:rtt -x rmemmgt.r
	{CC} -s rmemmgt {COptions} xrmemmgt.c
	rm xrmemmgt.c

xrmisc.c.o� rmisc.r {HDRS}
	:::bin:rtt -x rmisc.r
	{CC} -s rmisc {COptions} xrmisc.c
	rm xrmisc.c

xrstruct.c.o� rstruct.r {HDRS}
	:::bin:rtt -x rstruct.r
	{CC} -s rstruct {COptions} xrstruct.c
	rm xrstruct.c

xrsys.c.o� rsys.r {HDRS}
	:::bin:rtt -x rsys.r
	{CC} -s rsys {COptions} xrsys.c
	rm xrsys.c

xrwinrsc.c.o� rwinrsc.r {HDRS} {XICONHDRS}
	:::bin:rtt -x rwinrsc.r
	{CC} -s rwinrsc {COptions} xrwinrsc.c
	rm xrwinrsc.c

xrgfxsys.c.o� rgfxsys.r {HDRS} {XICONHDRS}
	:::bin:rtt -x rgfxsys.r
	{CC} -s rgfxsys {COptions} xrgfxsys.c
	rm xrgfxsys.c

xrwinsys.c.o� rwinsys.r {HDRS} {XICONHDRS}
	:::bin:rtt -x rwinsys.r
	{CC} -s rwinsys {COptions} xrwinsys.c
	rm xrwinsys.c

xrwindow.c.o� rwindow.r {HDRS} {XICONHDRS}
	:::bin:rtt -x rwindow.r
	{CC} -s rwindow {COptions} xrwindow.c
	rm xrwindow.c

xfxtra.c.o� fxtra.r {HDRS}
	:::bin:rtt -x fxtra.r
	{CC} -s fxtra {COptions} xfxtra.c
	rm xfxtra.c


####################################################################
#
# Make entries for the compiler library
#

comp_all�
	cd ::common; {MAKE} {ICOBJS} dlrgint.c.o
	{MAKE} db_lib

db_lib� rt.db rt.a

#
# if rt.db is missing or any header files have been updated, recreate
# rt.db from scratch along with the .c.o files.
#
rt.db� {HDRS}
	rm -f rt.db rt.a
	:::bin:rtt cnv.r data.r def.r errmsg.r fconv.r fmath.r �
	  fmisc.r fmonitr.r fscan.r fstr.r fstranl.r fstruct.r �
	  fsys.r fwindow.r init.r invoke.r keyword.r �
	  lmisc.r oarith.r oasgn.r ocat.r ocomp.r omisc.r �
	  oref.r oset.r ovalue.r ralc.r rcoexpr.r rcomp.r �
	  rdebug.r rlrgint.r rlocal.r rmemmgt.r rmisc.r rstruct.r �
	  rsys.r rwinrsc.r rgfxsys.r rwinsys.r rwindow.r fxtra.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rt.a� ::common:rswitch.a.o ::common:long.c.o ::common:time.c.o ::common:memory.c.o �
      cnv.c.o data.c.o def.c.o errmsg.c.o fconv.c.o fmath.c.o fmisc.c.o fmonitr.c.o fscan.c.o �
      fstr.c.o fstranl.c.o fstruct.c.o fsys.c.o fwindow.c.o init.c.o invoke.c.o �
      keyword.c.o lmisc.c.o oarith.c.o oasgn.c.o ocat.c.o ocomp.c.o omisc.c.o oref.c.o oset.c.o �
      ovalue.c.o ralc.c.o rcoexpr.c.o rcomp.c.o rdebug.c.o rlrgint.c.o rlocal.c.o rmemmgt.c.o �
      rmisc.c.o rstruct.c.o rsys.c.o rwinrsc.c.o rgfxsys.c.o rwinsys.c.o fxtra.c.o �
      rwindow.c.o ::common:xwindow.c.o
	rm -f rt.a
	ar qc rt.a `sed ':s:$$:.o:' rttfull.lst` ::common:rswitch.a.o �
	    ::common:long.c.o ::common:time.c.o ::common:memory.c.o �
	    ::common:xwindow.c.o
	cp rt.a rt.db ::common:dlrgint.c.o :::bin
	-(test -f :::NoRanlib) || (ranlib :::bin:rt.a)

cnv.c.o� cnv.r {HDRS}
	:::bin:rtt cnv.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

data.c.o� data.r {HDRS}
	:::bin:rtt data.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

def.c.o� def.r {HDRS}
	:::bin:rtt def.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

errmsg.c.o� errmsg.r {HDRS}
	:::bin:rtt errmsg.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fconv.c.o� fconv.r {HDRS}
	:::bin:rtt fconv.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fmath.c.o� fmath.r {HDRS}
	:::bin:rtt fmath.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fmisc.c.o� fmisc.r {HDRS}
	:::bin:rtt fmisc.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fmonitr.c.o� fmonitr.r {HDRS}
	:::bin:rtt fmonitr.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fscan.c.o� fscan.r {HDRS}
	:::bin:rtt fscan.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fstr.c.o� fstr.r {HDRS}
	:::bin:rtt fstr.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fstranl.c.o� fstranl.r {HDRS}
	:::bin:rtt fstranl.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fstruct.c.o� fstruct.r {HDRS}
	:::bin:rtt fstruct.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fsys.c.o� fsys.r {HDRS}
	:::bin:rtt fsys.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fwindow.c.o� fwindow.r {HDRS} {XICONHDRS}
	:::bin:rtt fwindow.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

init.c.o� init.r {HDRS}
	:::bin:rtt init.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

invoke.c.o� invoke.r {HDRS}
	:::bin:rtt invoke.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

keyword.c.o� keyword.r {HDRS}
	:::bin:rtt keyword.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

lmisc.c.o� lmisc.r {HDRS}
	:::bin:rtt lmisc.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

oarith.c.o� oarith.r {HDRS}
	:::bin:rtt oarith.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

oasgn.c.o� oasgn.r {HDRS}
	:::bin:rtt oasgn.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

ocat.c.o� ocat.r {HDRS}
	:::bin:rtt ocat.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

ocomp.c.o� ocomp.r {HDRS}
	:::bin:rtt ocomp.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

omisc.c.o� omisc.r {HDRS}
	:::bin:rtt omisc.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

oref.c.o� oref.r {HDRS}
	:::bin:rtt oref.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

oset.c.o� oset.r {HDRS}
	:::bin:rtt oset.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

ovalue.c.o� ovalue.r {HDRS}
	:::bin:rtt ovalue.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

ralc.c.o� ralc.r {HDRS}
	:::bin:rtt ralc.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rcoexpr.c.o� rcoexpr.r {HDRS}
	:::bin:rtt rcoexpr.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rcomp.c.o� rcomp.r {HDRS}
	:::bin:rtt rcomp.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rdebug.c.o� rdebug.r {HDRS}
	:::bin:rtt rdebug.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rlrgint.c.o� rlrgint.r {HDRS}
	:::bin:rtt rlrgint.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rlocal.c.o� rlocal.r {HDRS}
	:::bin:rtt rlocal.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rmemmgt.c.o� rmemmgt.r {HDRS}
	:::bin:rtt rmemmgt.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rmisc.c.o� rmisc.r {HDRS}
	:::bin:rtt rmisc.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rstruct.c.o� rstruct.r {HDRS}
	:::bin:rtt rstruct.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rsys.c.o� rsys.r {HDRS}
	:::bin:rtt rsys.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rwinrsc.c.o� rwinrsc.r {HDRS} {XICONHDRS}
	:::bin:rtt rwinrsc.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rgfxsys.c.o� rgfxsys.r {HDRS} {XICONHDRS}
	:::bin:rtt rgfxsys.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rwinsys.c.o� rwinsys.r {HDRS} {XICONHDRS}
	:::bin:rtt rwinsys.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

rwindow.c.o� rwindow.r {HDRS} {XICONHDRS}
	:::bin:rtt rwindow.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`

fxtra.c.o� fxtra.r {HDRS}
	:::bin:rtt fxtra.r
	{CC} {COptions} `sed ':s:$$:.c:' rttcur.lst`
	rm `sed ':s:$$:.c:' rttcur.lst`
