Incl = ":Include:"
ELoad="{EudoraObjs}eudora.load"
SLoad="{EudoraObjs}sysdefs.load"
#
COPTIONS = -y "{EudoraObjs}" -n -sym on -mbg on -r -i :Include -d EUDORA_LOAD=�""{Eload}�"" -d SYS_LOAD=�""{Sload}"�"
#
# Kerberos stuff
#
#COptions = -n -sym on -r -d KERBEROS -i "{netlibincl}" -i "{krbincl}"
#CPlusOptions = -d KERBEROS -sym on -i "{netlibincl}" -i "{krbincl}"
#RezOptions = -d KERBEROS
#
# add kerberos_glue.cp to SOURCES
# add kerberos_glue.cp.o to L1
#
#LIBS =  "{netlib}"dnr.c.o "{krblib}"krb.lib "{deslib}"des.lib �
#		"{netobjs}Machine Dependencies.cp.o" "{netobjs}UDPobj.cp.o"
#
SOURCES = �
  uupc.c comp.c compact.c boxact.c ends.c functions.c inet.c lineio.c �
  mailbox.c  main.c messact.c message.c mywindow.c shame.c sort.c �
  util.c globals.c tefuncs.c sendmail.c tcp.c mytcp.c dnr.c �
  progress.c cursor.c pop.c address.c print.c binhex.c �
  hexbin.c nickwin.c undo.c find.c modeless.c fileutil.c �
  winutil.c ph.c ctb.c text.c utl.c prefs.c mbwin.c �
  uudecode.c buildtoc.c squish.c md5.c scrollte.c lmgr.c nickmng.c �
	nickexp.c log.c unload.c menu.c

OBJECTS =  �
	{Objs}Unloadseg.c.o {Objs}address.c.o {Objs}binhex.c.o {Objs}boxact.c.o �
	{Objs}buildtoc.c.o {Objs}comp.c.o {Objs}compact.c.o {Objs}ctb.c.o �
	{Objs}cursor.c.o {Objs}dnr.c.o {Objs}ends.c.o {Objs}fileutil.c.o �
	{Objs}find.c.o {Objs}functions.c.o {Objs}globals.c.o {Objs}hexbin.c.o �
	{Objs}inet.c.o {Objs}lineio.c.o {Objs}lmgr.c.o {Objs}log.c.o �
	{Objs}mailbox.c.o {Objs}main.c.o {Objs}mbwin.c.o {Objs}md5.c.o {Objs}menu.c.o �
	{Objs}messact.c.o {Objs}message.c.o {Objs}modeless.c.o {Objs}mytcp.c.o �
	{Objs}mywindow.c.o {Objs}nickexp.c.o {Objs}nickmng.c.o {Objs}nickwin.c.o �
	{Objs}ph.c.o {Objs}pop.c.o {Objs}prefs.c.o {Objs}print.c.o �
	{Objs}progress.c.o {Objs}scrollte.c.o {Objs}sendmail.c.o {Objs}shame.c.o �
	{Objs}sort.c.o {Objs}squish.c.o {Objs}tcp.c.o {Objs}tefuncs.c.o �
	{Objs}text.c.o {Objs}undo.c.o {Objs}unload.c.o {Objs}util.c.o {Objs}utl.c.o �
	{Objs}uudecode.c.o {Objs}uupc.c.o {Objs}winutil.c.o

{Objs}biglib.o  �
	Lib -o {Objs}biglib.o -sym on "{Libraries}"Runtime.o "{Libraries}"Interface.o "{CLibraries}"StdCLib.o "{CLibraries}"Math.o "{Libraries}"PerformLib.o "{Libraries}"ToolLibs.o

INCLUDES = �
  {Incl}boxact.h {Incl}ends.h {Incl}functions.h {Incl}Globals.h �
	{Incl}inet.h {Incl}lineio.h {Incl}mailbox.h {Incl}main.h �
	{Incl}messact.h {Incl}message.h {Incl}MyRes.h {Incl}mywindow.h �
	{Incl}shame.h {Incl}sort.h {Incl}util.h {Incl}comp.h �
	{Incl}compact.h {Incl}tefuncs.h {Incl}sendmail.h {Incl}tcp.h �
  {Incl}mytcp.h {Incl}progress.h {Incl}cursor.h {Incl}pop.h �
	{Incl}print.h {Incl}nickmng.h {Incl}binhex.h {Incl}hexbin.h �
  {Incl}nickwin.h {Incl}find.h {Incl}modeless.h {Incl}undo.h �
	{Incl}fileutil.h {Incl}winutil.h {Incl}ph.h {Incl}ctb.h �
  {Incl}text.h {Incl}utl.h {Incl}buildtoc.h {Incl}squish.h �
	{Incl}prefs.h {Incl}uudecode.h {Incl}uupc.h {Incl}md5.h �
	{Incl}StringDefs.h {Incl}scrollte.h {Incl}lmgr.h {Incl}log.h �
	{Incl}numcode.h {Incl}nickexp.h {Incl}menu.h

RFILES_SRC = Eudora.r MacTcpErrors.r Navigate.r Version.r �
	 SMTP.r POP.r HelpStrns.r Other.r
RFILES = {RFILES_SRC} Strings.r  HelpMenuStrn.r HelpMenuHmnu.r  �
	 HelpDlogStrn.r HelpDlogHdlg.r

HMD = HelpMenuDefs
HMS = HelpMenuStrn.r
HMH = HelpMenuHmnu.r
HDD = HelpDlogDefs
HDS = HelpDlogStrn.r
HDH = HelpDlogHdlg.r

BIN_FILES = .pop.proj.rsrc kerberdora.rsrc
TEXT_FILES = {INCLUDES} {SOURCES} {RFILES_SRC} {HMD} {HDD} StringDefs

#
# Kerberos:  Add {LIBS} to the Link line
#
sade � {Objs}eudora.sym
{Objs}eudora.sym � {Objs}eudora
	Link -w -t APPL -c CSOm -sym on -mf �
		{OBJECTS} �
		{Objs}biglib.o �
		-o  {Objs}eudora
eudora � {Objs}eudora
{Objs}eudora �� {OBJECTS} {Objs}biglib.o
	Link -w -t APPL -c CSOm -sym off -mf �
		{OBJECTS} �
		{Objs}biglib.o �
		-o  {Objs}eudora
	SetFile -a BM {Objs}eudora
ldef.rsrc � {Objs}ldef.c.o
	Link -m LISTDEF -ra Purgeable -rn -rt LDEF=1001 -o ldef.rsrc {Objs}ldef.c.o {Objs}biglib.o
{Objs} � :
xref ��
	Link -w -mf -x eudora.xref -map -uf eudora.unref �
		{OBJECTS} �
		{Objs}biglib.o > eudora.map
	segalyze -v eudora.map eudora.xref >eudora.segs
	rm -y Link.Out
{Objs}eudora �� {RFILES} {OBJECTS}  .pop.proj.rsrc ldef.rsrc {Objs}biglib.o
	Rez -t APPL -c CSOm {RezOptions} �
                "{RIncludes}"SysTypes.r �
                "{RIncludes}"Types.r �
                "{RIncludes}"BalloonTypes.r �
		{RFILES} �
		-a -o  {Objs}eudora

{OBJECTS} � {ELoad}
{Eload} � MyDefs.c  {SLoad} {INCLUDES}
	{C} MyDefs.c -o {Objs}MyDefs.c.o {COPTIONS}
	Delete {Objs}MyDefs.c.o
{SLoad} � SysDefs.c
	{C} SysDefs.c {COPTIONS} -o {Objs}SysDefs.c.o
	Delete {Objs}SysDefs.c.o
 
{Incl}StringDefs.h � StringDefs
	:Bits:ProcessStrings StringDefs {incl}StringDefs.h Strings.r Strings.doc

{Incl}Globals.h � Globals.c
	perl -e '<>;<>;while(<>){s/^[A-Za-z]+/extern $&/;s/ = .*/;/;print;}' Globals.c >{Incl}Globals.h

segs �
	perl :Bits:makesegs Unloadseg.c {Incl}numcode.h {SOURCES}

Version.r � :Bits:progversion :Bits:prefversion :Bits:version.tmpl
	perl -p -e  "s/PROGVERSNUMBER/`cat :Bits:progversion`/g; s/PREFVERSNUMBER/`cat :Bits:prefversion`/g;" �
	    :Bits:version.tmpl >Version.r

version �
	perl -p -i~ -e 'chop;$_++;$_.="\n";' :Bits:progversion
	delete ':Bits:progversion~'

prefv �
	perl -p -i~ -e 'chop;$_++;$_.="\n";' :Bits:prefversion

{HDH} {HDS}�	{HDD}
	:Bits:ProcessDlogHelp {HDD} {HDH} {HDS} 17000

{HMH} {HMS}�	{HMD}
	:Bits:ProcessMenuHelp {HMD} {HMH} {HMS} 16000

tags � {SOURCES} {INCLUDES}
	CTags -local -update -h -i :Include {NewerDeps}

clean �
	rm -y -i eudora tags 
	ls | rmext ~ .makeout .xref .unref .load .map .segs .tmp .usf .SYM .o .dumpobj .l

rcs � rcsbin rcstext
	date > rcs

rcsbin � {BIN_FILES}
	ci -b {NewerDeps}
	date > rcsbin

rcstext � {TEXT_FILES}
	ci -t {NewerDeps}
	date > rcstext

printc � {SOURCES}
	obprint {NewerDeps}
	print -c 4 -s 10 �.l
	rm �.l �.dumpobj
	perl :bits:makeindex {SOURCES} | print -c 4 -s 12
	date > printc

stampversion � version.r
	open -t "version.r"
	find �3
	find /�"/
	find /[¶"]+/
	set vers `catenate �`
	close "version.r"
	NameRevisions -a "Eudora{vers}"

readme �
	:Bits:ftp_README >pub:mac:eudora:README.first
	:Bits:intl_README >pub:mac:eudora:intl:README.intl
	:Bits:mandisk_README >documents:eudora:manualdisk:README.manual
	:Bits:progdisk_README >documents:eudora:programdisk:README.first
	:Bits:source_README >pub:mac:eudora:source:README.source
	:Bits:contrib_README >pub:mac:eudora:contrib:README.contrib
	:Bits:contrib_README >documents:eudora:programdisk:contributed:README.contrib
