# This is a complete MPW makefile for Nethack and all its associated files
# Requires MPW C3.2�3 or later. Earlier versions will fail horribly because
# they can't cope with /* /* */ comments
#
# For NetHack 3.1 Nov 1991
#
# by Michael� Hamel and Ross Brown 1991 : michael@otago.ac.nz

#--------------------------------------------------------------------------
#
# BEFORE YOU BUILD FOR THE FIRST TIME
#  Set this equate to the folder containing all the NetHack source folders

Top   	   = {MPW}nh31:

#  Set the default directory to {Top}
#  Move NetHack.make into {Top}
#  Make two new empty folders in top:
#    - Obj for the object files
#    - Dungeon for the completed game
#  Ensure that you have at least 7000K allocated to the MPW Shell
#
#--------------------------------------------------------------------------

# Set up symbols for folders from the distribution
Src   	   = {Top}Src:
Util 	   = {Top}Util:
Dat		   = {Top}Dat:
Include    = {Top}Include:
MacDir	   = {Top}Sys:Mac:
TtyDir     = {Top}Win:Tty:
MacTty     = {Top}Sys:Mac:
Share	   = {Top}Sys:Share:

# These two folders are new
ObjDir     = {Top}Obj:				# "Temporary" stuff
Results	   = {Top}Dungeon:			# Where the game goes

# Override the settings in "{Include}"config.h
NHConfig = -d SCORE_ON_BOTL -d VISION_TABLES

# Type and creator - note that we use "PREF" for all non-save files
# currently. Since we will integrate the data files into the game
# sooner or later, this doesn't matter much.
FileType   = PREF
FileCreator= nh31

# NetHack is an abstract target which in fact consists of:

SpecialLevels = "{Results}"BigRoom.lev  �
				"{Results}"Castle.lev   �
				"{Results}"air.lev		�
				"{Results}"Medusa-1.lev �
				"{Results}"Quest.Dat 	�
				"{Results}"Valley.lev	�
				"{Results}"Oracle.lev   �
				"{Results}"Tower1.lev   �
				"{Results}"A-start.lev  �
				"{Results}"B-start.lev  �
				"{Results}"C-start.lev  �
				"{Results}"E-start.lev  �
				"{Results}"H-start.lev  �
				"{Results}"K-start.lev  �
				"{Results}"P-start.lev  �
				"{Results}"R-start.lev  �
				"{Results}"S-start.lev  �
				"{Results}"T-start.lev  �
				"{Results}"V-start.lev  �
				"{Results}"W-start.lev  �
				"{Results}"Mine_End.lev �
				"{Results}"knox.lev		�
				"{Results}"wizard1.lev

DataFiles 	=	"{Results}"Dungeon	�
				"{Results}"Record   �
				"{Results}"NetHack� Defaults   �
				"{Results}"Data 	�
				"{Results}"Rumors	�
				"{Results}"Oracles	�
				"{Results}"Cmdhelp
				
NetHack �   {SpecialLevels} {DataFiles} "{Results}"NetHack 

# These files are seperate on other machines, but here they are built
# into the application by NetHack.r

BuiltInData =   "{Dat}"hh 			�
				"{Dat}"History 		�
				"{Dat}"License 		�
				"{Dat}"Help 		�
				"{Dat}"Wizhelp 		�
#				"{Dat}"Cmdhelp 		�
				"{Dat}"Opthelp		�
				"{MacDir}"Machelp	�
				"{MacDir}"News		�
				"{ObjDir}"Options
						   

# For debugging: if you change this you need to discard everything in {Objs}
SADEOptions = # -sym full

# <32K file-level segmentation scheme
LinkType = -mf -srt 											�
		   -sg Main=allmain,macmain,random,rnd,STDCLIB,STDIO	�
		   -sg ARes=apply										�
		   -sg BRes=botl,display,vision							�
		   -sg CRes=do_wear,cmd,track							�
		   -sg DRes=do											�
		   -sg ERes=dog,dogmove									�
		   -sg FRes=eat,mttymain,mactty							�
		   -sg GRes=engrave,dungeon,dbridge						�
		   -sg HRes=hack,hacklib								�
		   -sg IRes=invent										�
		   -sg JRes=monmove,mon									�
		   -sg KRes=mondata,mthrowu,muse,were,wizard			�
		   -sg LRes=macwin,wintty								�
		   -sg MRes=macmenu								�
		   -sg NRes=pickup,sounds,vault							�
		   -sg ORes=quest,trap									�
		   -sg PRes=timeout,attrib,lock
BuildType = -m -s {Default} 	# Make a new segment for each file by default 

# Alternatively you can use 
# LinkType = -model far
# BuildType = -model far -d MODEL_FAR

# We use the default rule that .c.o files depend on .c files: set up these
# directory variables so it knows where to find them

"{ObjDir}" � "{Src}" "{Util}" "{MacDir}" "{Share}" "{TtyDir}" "{MacTty}"
  
# Compiler options. We set up to look in the Mac dir first for include files
# so we can take over hack.h and use a dump file
COptions = -r 							# Warn about undefined functions 		�
		   -i "{MacDir}","{Include}" 	# Where to look for include files 		�
		   {BuildType}					�
		   -mbg full					�
		   {NHConfig}					�
		   {SADEOptions}

NetHackLink = {LinkType} -c 'nh31' -t APPL -mf �
			  {SADEOptions}   #  -map >link.map 

MPWToolLink	= -br on -srt -d -c 'MPS ' -t MPST -sn %A5Init=Init

 
# ------------ The include files depend on one another ---------------
# Rather than touching them which makes it look as though they have changed
# when they haven't and generates unnecessary backups, we define symbols:

macconf.h = "{Include}"macconf.h "{Include}"system.h

pcconf.h = "{Include}"pcconf.h "{Include}"micro.h "{Include}"system.h

global.h = "{Include}"global.h "{Include}"coord.h {pcconf.h} {macconf.h}

config.h = "{Include}"config.h "{Include}"tradstdc.h {global.h}

youprop.h = "{Include}"youprop.h "{Include}"prop.h {permonst.h} "{Include}"mondata.h �
			"{Include}"pm.h

you.h = "{Include}"you.h "{Include}"attrib.h "{Include}"monst.h {youprop.h}

decl.h	= "{Include}"decl.h	"{Include}"spell.h "{Include}"color.h "{Include}"obj.h �
		  {you.h} "{Include}"onames.h

display.h = "{Include}"display.h "{Include}"vision.h "{Include}"mondata.h

emin.h = "{Include}"emin.h "{Include}"dungeon.h

epri.h = "{Include}"epri.h "{Include}"dungeon.h

eshk.h = "{Include}"eshk.h "{Include}"dungeon.h


permonst.h = "{Include}"permonst.h 	"{Include}"monattk.h "{Include}"monflag.h

vault.h = "{Include}"vault.h "{Include}"dungeon.h
			
#------------------- Use a dump file for hack.h to speed compiles -----------------
# We do this by having our own hack.h in :sys:mac which just grabs the dump file
# from Obj. The dependencies are set up to build the dump file if its missing

realhack.h = "{Include}"hack.h {config.h} "{Include}"dungeon.h {decl.h} �
			 "{Include}"monsym.h "{Include}"mkroom.h "{Include}"objclass.h �
			 "{Include}"trap.h "{Include}"flag.h "{Include}"rm.h �
			 {display.h} "{Include}"wintype.h "{Include}"engrave.h �
			 "{Include}"rect.h  "{Include}"trampoli.h "{Include}"extern.h 

hack.h = "{ObjDir}"hack.hdump

# This compile is done solely for the side effect of generating hack.hdump
"{ObjDir}"hack.hdump � {realhack.h} "{MacDir}"mhdump.c
	C -i "{Include}" {NHConfig} {SADEOptions} �
	  "{MacDir}"mhdump.c -o "{ObjDir}"mhdump.c.o


"{Include}"lev_comp.h � "{Share}"lev_comp.h
	duplicate -y "{Share}"lev_comp.h "{Include}"lev_comp.h

"{Include}"dgn_comp.h � "{Share}"dgn_comp.h
	duplicate -y "{Share}"dgn_comp.h "{Include}"dgn_comp.h

#------------------- NetHack objects -----------------

CObjs = "{ObjDir}"allmain.c.o �
        "{ObjDir}"alloc.c.o �
        "{ObjDir}"apply.c.o �
        "{ObjDir}"artifact.c.o �
        "{ObjDir}"attrib.c.o �
		"{ObjDir}"ball.c.o �
        "{ObjDir}"bones.c.o �
        "{ObjDir}"botl.c.o �
        "{ObjDir}"cmd.c.o �
        "{ObjDir}"dbridge.c.o �
        "{ObjDir}"decl.c.o �
		"{ObjDir}"detect.c.o �
        "{ObjDir}"display.c.o �
        "{ObjDir}"do.c.o �
        "{ObjDir}"do_name.c.o �
        "{ObjDir}"do_wear.c.o �
        "{ObjDir}"dog.c.o �
        "{ObjDir}"dogmove.c.o �
        "{ObjDir}"dokick.c.o �
        "{ObjDir}"dothrow.c.o �
		"{ObjDir}"drawing.c.o �
        "{ObjDir}"dungeon.c.o �
        "{ObjDir}"eat.c.o �
        "{ObjDir}"end.c.o �
        "{ObjDir}"engrave.c.o �
        "{ObjDir}"exper.c.o �
        "{ObjDir}"explode.c.o �
        "{ObjDir}"extralev.c.o �
        "{ObjDir}"files.c.o �
        "{ObjDir}"fountain.c.o �
        "{ObjDir}"hack.c.o �
        "{ObjDir}"hacklib.c.o �
        "{ObjDir}"invent.c.o �
        "{ObjDir}"lock.c.o �
        "{ObjDir}"maccurs.c.o �
		"{ObjDir}"macerrs.c.o �
        "{ObjDir}"macfile.c.o �
        "{ObjDir}"macmain.c.o �
        "{ObjDir}"macmenu.c.o �
		"{ObjDir}"macsnd.c.o �
        "{ObjDir}"macunix.c.o �
        "{ObjDir}"macwin.c.o �
        "{ObjDir}"makemon.c.o �
        "{ObjDir}"mcastu.c.o �
        "{ObjDir}"mgetline.c.o �
        "{ObjDir}"mhitm.c.o �
        "{ObjDir}"mhitu.c.o �
        "{ObjDir}"minion.c.o �
        "{ObjDir}"mklev.c.o �
		"{ObjDir}"mkmap.c.o �
        "{ObjDir}"mkmaze.c.o �
        "{ObjDir}"mkobj.c.o �
        "{ObjDir}"mkroom.c.o �
        "{ObjDir}"mmodal.c.o �
        "{ObjDir}"mon.c.o �
        "{ObjDir}"mondata.c.o �
        "{ObjDir}"monmove.c.o �
        "{ObjDir}"monst.c.o �
		"{ObjDir}"monstr.c.o �
		"{ObjDir}"mplayer.c.o �
		"{ObjDir}"mstring.c.o �
        "{ObjDir}"mthrowu.c.o �
        "{ObjDir}"muse.c.o �
        "{ObjDir}"music.c.o �
        "{ObjDir}"o_init.c.o �
        "{ObjDir}"objects.c.o �
        "{ObjDir}"objnam.c.o �
        "{ObjDir}"options.c.o �
        "{ObjDir}"pager.c.o �
        "{ObjDir}"pickup.c.o �
        "{ObjDir}"pline.c.o �
        "{ObjDir}"polyself.c.o �
        "{ObjDir}"potion.c.o �
        "{ObjDir}"pray.c.o �
        "{ObjDir}"priest.c.o �
		"{ObjDir}"quest.c.o �
		"{ObjDir}"questpgr.c.o �
        "{ObjDir}"random.c.o �
        "{ObjDir}"read.c.o �
        "{ObjDir}"rect.c.o �
        "{ObjDir}"restore.c.o �
        "{ObjDir}"rip.c.o �
        "{ObjDir}"rnd.c.o �
        "{ObjDir}"rumors.c.o �
        "{ObjDir}"save.c.o �
        "{ObjDir}"shk.c.o �
        "{ObjDir}"shknam.c.o �
        "{ObjDir}"sit.c.o �
        "{ObjDir}"sounds.c.o �
        "{ObjDir}"sp_lev.c.o �
        "{ObjDir}"spell.c.o �
        "{ObjDir}"steal.c.o �
        "{ObjDir}"timeout.c.o �
        "{ObjDir}"mactopl.c.o �
        "{ObjDir}"topten.c.o �
        "{ObjDir}"track.c.o �
        "{ObjDir}"trap.c.o �
        "{ObjDir}"u_init.c.o �
        "{ObjDir}"uhitm.c.o �
        "{ObjDir}"vault.c.o �
        "{ObjDir}"version.c.o �
        "{ObjDir}"vision.c.o �
        "{ObjDir}"vis_tab.c.o �
        "{ObjDir}"weapon.c.o �
        "{ObjDir}"were.c.o �
        "{ObjDir}"wield.c.o �
		"{ObjDir}"windows.c.o �
        "{ObjDir}"wizard.c.o �
        "{ObjDir}"worm.c.o �
        "{ObjDir}"worn.c.o �
        "{ObjDir}"write.c.o �
        "{ObjDir}"zap.c.o �
		�
		"{ObjDir}"getline.c.o �
		"{ObjDir}"topl.c.o �
		"{ObjDir}"wintty.c.o �
		�
		"{ObjDir}"mactty.c.o �
		"{ObjDir}"mttymain.c.o �
		�
		"{CLibraries}"StdClib.o �
		"{Libraries}"Runtime.o �
		"{Libraries}"Interface.o �
		�
		"{ObjDir}"dprintf.c.o

#------------- The object files depend upon the include files as follows -----

"{ObjDir}"allmain.c.o	�  {hack.h}
"{ObjDir}"alloc.c.o  	�  {config.h}
"{ObjDir}"apply.c.o		�  {hack.h} "{Include}"edog.h
"{ObjDir}"artifact.c.o	�  {hack.h} "{Include}"artifact.h "{Include}"artilist.h
"{ObjDir}"attrib.c.o	�  {hack.h} "{Include}"artifact.h
"{ObjDir}"ball.c.o		�  {hack.h}
"{ObjDir}"bones.c.o		�  {hack.h} "{Include}"lev.h
"{ObjDir}"botl.c.o		�  {hack.h}
"{ObjDir}"cmd.c.o		�  {hack.h} "{Include}"func_tab.h
"{ObjDir}"dbridge.c.o	�  {hack.h}
"{ObjDir}"decl.c.o		�  {hack.h} "{Include}"quest.h
"{ObjDir}"detect.c.o	�  {hack.h} "{Include}"artifact.h
"{ObjDir}"display.c.o	�  {hack.h}
"{ObjDir}"do.c.o		�  {hack.h} "{Include}"lev.h
"{ObjDir}"do_name.c.o	�  {hack.h}
"{ObjDir}"do_wear.c.o	�  {hack.h}
"{ObjDir}"dog.c.o		�  {hack.h} "{Include}"edog.h
"{ObjDir}"dogmove.c.o	�  {hack.h} "{Include}"mfndpos.h "{Include}"edog.h
"{ObjDir}"dokick.c.o	�  {hack.h} {eshk.h}
"{ObjDir}"dothrow.c.o	�  {hack.h}
"{ObjDir}"drawing.c.o	�  {hack.h} "{Include}"termcap.h
"{ObjDir}"dungeon.c.o	�  {hack.h} "{Include}"dgn_file.h
"{ObjDir}"eat.c.o		�  {hack.h}
"{ObjDir}"end.c.o		�  {hack.h} {eshk.h}
"{ObjDir}"engrave.c.o	�  {hack.h} "{Include}"lev.h
"{ObjDir}"exper.c.o		�  {hack.h}
"{ObjDir}"explode.c.o	�  {hack.h}
"{ObjDir}"extralev.c.o	�  {hack.h}
"{ObjDir}"files.c.o		�  {hack.h}
"{ObjDir}"fountain.c.o	�  {hack.h}
"{ObjDir}"hack.c.o		�  {hack.h}
"{ObjDir}"hacklib.c.o	�  {config.h}
"{ObjDir}"invent.c.o	�  {hack.h} "{Include}"artifact.h
"{ObjDir}"lock.c.o		�  {hack.h}
"{ObjDir}"macmain.c.o	�  {hack.h}
"{ObjDir}"macmenu.c.o	�  {hack.h} "{Include}"patchlevel.h
"{ObjDir}"mmodal.c.o	�  {hack.h}
"{ObjDir}"macsnd.c.o	�  {hack.h}
"{ObjDir}"macwin.c.o	�  {hack.h} "{Include}"mactty.h "{Include}"wintty.h "{Include}"func_tab.h
"{ObjDir}"maccurs.c.o	�  {hack.h}
"{ObjDir}"macfile.c.o	�  {hack.h}
"{ObjDir}"macerrs.c.o	�  {hack.h}
"{ObjDir}"mactopl.c.o	�  {hack.h}
"{ObjDir}"macunix.c.o	�  {hack.h}
"{ObjDir}"mail.c.o		�  {hack.h}
"{ObjDir}"makemon.c.o	�  {hack.h} {epri.h} {emin.h}
"{ObjDir}"mcastu.c.o	�  {hack.h}
"{ObjDir}"mgetline.c.o	�  {hack.h}
"{ObjDir}"mhitm.c.o		�  {hack.h} "{Include}"artifact.h "{Include}"edog.h
"{ObjDir}"mhitu.c.o		�  {hack.h} "{Include}"artifact.h "{Include}"edog.h
"{ObjDir}"minion.c.o	�  {hack.h} {emin.h} {epri.h}
"{ObjDir}"mklev.c.o		�  {hack.h}
"{ObjDir}"mkmap.c.o		�  {hack.h} "{Include}"sp_lev.h
"{ObjDir}"mkmaze.c.o	�  {hack.h}
"{ObjDir}"mkobj.c.o		�  {hack.h} "{Include}"artifact.h "{Include}"prop.h
"{ObjDir}"mkroom.c.o	�  {hack.h}
"{ObjDir}"mon.c.o		�  {hack.h} "{Include}"mfndpos.h "{Include}"edog.h
"{ObjDir}"mondata.c.o	�  {hack.h} {eshk.h} {epri.h}
"{ObjDir}"monmove.c.o	�  {hack.h} "{Include}"mfndpos.h "{Include}"artifact.h
"{ObjDir}"monst.c.o		�  {config.h} {permonst.h} "{Include}"monsym.h �
						   {eshk.h} {vault.h} {epri.h}�
						   "{Include}"color.h
"{ObjDir}"mplayer.c.o	�  {hack.h}
"{ObjDir}"mthrowu.c.o	�  {hack.h}
"{ObjDir}"muse.c.o		�  {hack.h}
"{ObjDir}"music.c.o		�  {hack.h}
"{ObjDir}"o_init.c.o	�  {hack.h}
"{ObjDir}"objects.c.o	�  {config.h} "{Include}"obj.h "{Include}"objclass.h�
						   "{Include}"prop.h "{Include}"color.h
"{ObjDir}"objnam.c.o	�  {hack.h}
"{ObjDir}"options.c.o	�  {hack.h} "{Include}"termcap.h
"{ObjDir}"pager.c.o		�  {macconf.h}
"{ObjDir}"panic.c.o		�  {config.h}
"{ObjDir}"pickup.c.o	�  {hack.h}
"{ObjDir}"pline.c.o		�  {hack.h} {epri.h}
"{ObjDir}"polyself.c.o	�  {hack.h}
"{ObjDir}"potion.c.o	�  {hack.h}
"{ObjDir}"pray.c.o		�  {hack.h} {epri.h}
"{ObjDir}"priest.c.o	�  {hack.h} "{Include}"mfndpos.h {eshk.h}�
						   {epri.h} {emin.h}
"{ObjDir}"quest.c.o		�  {hack.h} "{Include}"quest.h "{Include}"qtext.h
"{ObjDir}"questpgr.c.o	�  {hack.h} "{Include}"qtext.h
"{ObjDir}"read.c.o		�  {hack.h}
"{ObjDir}"rect.c.o		�  {hack.h}
"{ObjDir}"restore.c.o	�  {hack.h} "{Include}"lev.h "{Include}"termcap.h "{Include}"quest.h
"{ObjDir}"rip.c.o		�  {hack.h}
"{ObjDir}"rnd.c.o		�  {hack.h}
"{ObjDir}"rumors.c.o	�  {hack.h}
"{ObjDir}"save.c.o		�  {hack.h} "{Include}"lev.h "{Include}"quest.h
"{ObjDir}"shk.c.o		�  {hack.h} {eshk.h}
"{ObjDir}"shknam.c.o	�  {hack.h} {eshk.h}
"{ObjDir}"sit.c.o		�  {hack.h} "{Include}"artifact.h
"{ObjDir}"sounds.c.o	�  {hack.h} "{Include}"edog.h
"{ObjDir}"sp_lev.c.o	�  {hack.h} "{Include}"sp_lev.h "{Include}"rect.h
"{ObjDir}"spell.c.o		�  {hack.h}
"{ObjDir}"steal.c.o		�  {hack.h}
"{ObjDir}"timeout.c.o	�  {hack.h}
"{ObjDir}"topten.c.o	�  {hack.h}
"{ObjDir}"track.c.o		�  {hack.h}
"{ObjDir}"trap.c.o		�  {hack.h}
"{ObjDir}"u_init.c.o	�  {hack.h}
"{ObjDir}"uhitm.c.o		�  {hack.h}
"{ObjDir}"vault.c.o		�  {hack.h} {vault.h}
"{ObjDir}"version.c.o	�  {hack.h} "{Include}"date.h  "{Include}"patchlevel.h "{Include}"termcap.h
"{ObjDir}"vision.c.o	�  {hack.h} "{Include}"vis_tab.h
"{ObjDir}"weapon.c.o	�  {hack.h}
"{ObjDir}"were.c.o		�  {hack.h}
"{ObjDir}"wield.c.o		�  {hack.h}
"{ObjDir}"windows.c.o	�  {hack.h}
"{ObjDir}"wizard.c.o	�  {hack.h} "{Include}"qtext.h
"{ObjDir}"worm.c.o		�  {hack.h} "{Include}"lev.h
"{ObjDir}"worn.c.o		�  {hack.h}
"{ObjDir}"write.c.o		�  {hack.h}
"{ObjDir}"zap.c.o		�  {hack.h}

"{ObjDir}"getline.c.o	�	{hack.h} "{Include}"wintty.h "{Include}"func_tab.h
"{ObjDir}"topl.c.o		�	{hack.h} "{Include}"wintty.h "{Include}"termcap.h
"{ObjDir}"wintty.c.o 	�	{hack.h} "{Include}"wintty.h "{Include}"termcap.h

"{ObjDir}"mactty.c.o	�	{hack.h} "{Include}"mactty.h "{Include}"mttypriv.h
"{ObjDir}"mttymain.c.o 	�	{hack.h} "{Include}"mactty.h

"{ObjDir}"MakeDefs.c.o  �  {config.h} {permonst.h} "{Include}"objclass.h �
						   "{Include}"monsym.h "{Include}"artilist.h �
						   "{Include}"patchlevel.h "{Include}"qtext.h

"{ObjDir}"dgn_yacc.c.o	�  {config.h} "{Include}"dgn_file.h
"{ObjDir}"dgn_lex.c.o	�  {config.h} "{Include}"dgn_comp.h "{Include}"dgn_file.h
"{ObjDir}"dgn_main.c.o	�  {config.h}

"{ObjDir}"lev_yacc.c.o	�  {hack.h} "{Include}"sp_lev.h
"{ObjDir}"lev_lex.c.o	�  {hack.h} "{Include}"lev_comp.h "{Include}"sp_lev.h
"{ObjDir}"lev_main.c.o	�  {hack.h} "{Include}"sp_lev.h

"{ObjDir}"dprintf.c.o	�	{hack.h}

# -------- Build the dungeon compiler, as an MPW tool ---------------
	
DgnObjs= "{ObjDir}"dgn_lex.c.o		�
		 "{ObjDir}"dgn_main.c.o		�
		 "{ObjDir}"dgn_yacc.c.o		�
		 "{ObjDir}"alloc.c.o		�
		 "{ObjDir}"panic.c.o		�
		 "{Libraries}"Stubs.o		�
		 "{Libraries}"Runtime.o 	�
		 "{Libraries}"ToolLibs.o	�
		 "{Libraries}"Interface.o	�
		 "{CLibraries}"StdCLib.o

DgnComp	� {DgnObjs}
	Link {MPWToolLink}  �
		 {DgnObjs} -o {Targ} �
#		  -sg Main=alloc,dgn_lex,dgn_main,dgn_yacc,panic,STDIO,INTENV,SADEV �
		
# -------- Build the special-level compiler, as an MPW tool ---------------
	
LevObjs= "{ObjDir}"monst.c.o		�
		 "{ObjDir}"objects.c.o		�
		 "{ObjDir}"drawing.c.o		�
		 "{ObjDir}"alloc.c.o		�
		 "{ObjDir}"panic.c.o		�
		 "{ObjDir}"lev_lex.c.o		�
		 "{ObjDir}"lev_yacc.c.o		�
		 "{ObjDir}"macfile.c.o		�
		 "{ObjDir}"macerrs.c.o		�
		 "{ObjDir}"files.c.o		�
		 "{ObjDir}"decl.c.o			�
		 "{Libraries}"Stubs.o		�
		 "{ObjDir}"lev_main.c.o		�
		 "{CLibraries}"StdCLib.o	�
		  "{Libraries}"ToolLibs.o	�
		 "{Libraries}"Runtime.o 	�
		 "{Libraries}"Interface.o  

LevComp	� {LevObjs}
	Link {MPWToolLink} �
		 {LevObjs} -o {Targ} �
#		 -sg Main=objects,monst,macfile,files,macerrs,lev_lex,lev_main,lev_yacc �
		
# -------- Build MakeDefs, as an MPW tool ---------------

MakeDefsObjs= "{ObjDir}"objects.c.o		�
			  "{ObjDir}"monst.c.o		�
			  "{ObjDir}"MakeDefs.c.o	�
			  "{Libraries}"Stubs.o		�
    		  "{Libraries}"Runtime.o 	�
			  "{Libraries}"Interface.o	�
			  "{Libraries}"ToolLibs.o	�
			  "{CLibraries}"StdCLib.o

MakeDefs	�� {MakeDefsObjs}
	Link {MPWToolLink} �
		 {MakeDefsObjs} -o {Targ}
	
		
# ------------ If MakeDefs changes we need to rebuild some include files -----------

"{Include}"date.h 	� 	MakeDefs
	MakeDefs -v
	Move -y "{Dat}"Options "{ObjDir}"

"{ObjDir}"Options	�	"{Include}"date.h

"{Include}"onames.h �	MakeDefs
	MakeDefs -o

"{Include}"pm.h 	�	MakeDefs
	MakeDefs -p

"{Src}"vis_tab.c	�	MakeDefs "{Include}"vis_tab.h

"{Include}"vis_tab.h �	MakeDefs
	MakeDefs -z
	
"{ObjDir}"Dungeon.pdf  � "{Dat}"Dungeon.def MakeDefs
	MakeDefs -e
	Move -y "{Dat}"Dungeon.pdf "{ObjDir}"
	
"{Src}"monstr.c � Makedefs {config.h}
	MakeDefs -m

#---------------- Main dependencies and the application, at last -------------------

"{Results}"Data  � "{Dat}"Data.base MakeDefs
		MakeDefs -d
		Move -y "{Dat}"Data "{Results}"
		SetFile -t "{FileType}" -c "{FileCreator}" "{Results}"Data

"{Results}"Rumors  � "{Dat}"Rumors.tru "{Dat}"Rumors.fal MakeDefs
		MakeDefs -r
		Move -y "{Dat}"Rumors "{Results}"
		SetFile -t "{FileType}" -c "{FileCreator}" "{Results}"Rumors

"{Results}"Oracles � "{Dat}"Oracles.txt MakeDefs
		MakeDefs -h
		Move -y "{Dat}"Oracles "{Results}"
		SetFile -t "{FileType}" -c "{FileCreator}" "{Results}"Oracles

"{Results}"Cmdhelp � "{Dat}"Cmdhelp
		duplicate -y "{Dat}"Cmdhelp "{Results}"
		SetFile -t "{FileType}" -c "{FileCreator}" "{Results}"Cmdhelp

"{Results}"Record  � 
		Open -n -t "{Results}"Record
		Replace /�/ "This is the record file"
		Close -y "{Results}"Record

"{Results}"NetHack� Defaults  � 
		duplicate -y "{MacDir}"NHDeflts "{Results}"NetHack� Defaults

"{Results}"Dungeon  � "{ObjDir}"Dungeon.pdf DgnComp
		DgnComp "{ObjDir}"dungeon.pdf
		Move -y "{ObjDir}"Dungeon "{Results}"
		SetFile -t "{FileType}" -c "{FileCreator}" "{Results}"Dungeon

"{Results}"BigRoom.lev  � "{Dat}"BigRoom.des LevComp
		LevComp "{Dat}"BigRoom.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y BigRoom.lev "{Results}"

"{Results}"Castle.lev  � "{Dat}"Castle.des LevComp
		LevComp "{Dat}"Castle.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y Castle.lev "{Results}"

"{Results}"air.lev		� "{dat}"endgame.des levcomp
	LevComp "{dat}"endgame.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
	Move -y air.lev "{results}"
	Move -y astral.lev "{Results}"
	Move -y earth.lev "{Results}"
	Move -y fire.lev "{Results}"
	Move -y water.lev "{Results}"

"{Results}"Medusa-1.lev � "{Dat}"Medusa.des LevComp
		LevComp "{Dat}"Medusa.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y Medusa-1.lev "{Results}"
		Move -y Medusa-2.lev "{Results}"

"{Results}"Oracle.lev  � "{Dat}"Oracle.des LevComp
		LevComp "{Dat}"Oracle.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y Oracle.lev "{Results}"

"{Results}"Mine_End.lev  � "{Dat}"Mines.des LevComp
		LevComp "{Dat}"Mines.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y Mine_End.lev "{Results}" 
		Move -y MineFill.lev "{Results}"
		Move -y MineTown.lev "{Results}"

"{Results}"Tower1.lev  � "{Dat}"Tower.des LevComp
		LevComp "{Dat}"Tower.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y Tower1.lev "{Results}"
		Move -y Tower2.lev "{Results}"
		Move -y Tower3.lev "{Results}"

"{Results}"knox.lev � "{Dat}"knox.des LevComp
		LevComp "{Dat}"knox.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y knox.lev "{Results}"

"{Results}"wizard1.lev � "{Dat}"yendor.des LevComp
		LevComp "{Dat}"yendor.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y wizard1.lev "{Results}"
		Move -y wizard2.lev "{Results}"
		Move -y wizard3.lev "{Results}"
		Move -y fakewiz1.lev "{Results}"
		Move -y fakewiz2.lev "{Results}"

"{Results}"A-start.lev  � "{Dat}"Arch.des LevComp
		LevComp "{Dat}"Arch.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y A-fill�.lev	"{Results}"
		Move -y A-goal.lev "{Results}"
		Move -y A-locate.lev "{Results}"
		Move -y A-start.lev "{Results}"

"{Results}"B-start.lev  � "{Dat}"Barb.des LevComp
		LevComp "{Dat}"Barb.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y B-fill�.lev	"{Results}"
		Move -y B-goal.lev "{Results}"
		Move -y B-locate.lev "{Results}"
		Move -y B-start.lev "{Results}"

"{Results}"C-start.lev  � "{Dat}"Caveman.des LevComp
		LevComp "{Dat}"Caveman.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y C-fill�.lev	"{Results}"
		Move -y C-goal.lev "{Results}"
		Move -y C-locate.lev "{Results}"
		Move -y C-start.lev "{Results}"

"{Results}"E-start.lev  � "{Dat}"Elf.des LevComp
		LevComp "{Dat}"Elf.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y E-fill�.lev	"{Results}"
		Move -y E-goal.lev "{Results}"
		Move -y E-locate.lev "{Results}"
		Move -y E-start.lev "{Results}"

"{Results}"H-start.lev  � "{Dat}"Healer.des LevComp
		LevComp "{Dat}"Healer.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y H-fill�.lev	"{Results}"
		Move -y H-goal.lev "{Results}"
		Move -y H-locate.lev "{Results}"
		Move -y H-start.lev "{Results}"

"{Results}"K-start.lev  � "{Dat}"Knight.des LevComp
		LevComp "{Dat}"Knight.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y K-fill�.lev	"{Results}"
		Move -y K-goal.lev "{Results}"
		Move -y K-locate.lev "{Results}"
		Move -y K-start.lev "{Results}"

"{Results}"P-start.lev  � "{Dat}"Priest.des LevComp
		LevComp "{Dat}"Priest.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y P-fill�.lev	"{Results}"
		Move -y P-goal.lev "{Results}"
		Move -y P-locate.lev "{Results}"
		Move -y P-start.lev "{Results}"

"{Results}"R-start.lev  � "{Dat}"Rogue.des LevComp
		LevComp "{Dat}"Rogue.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y R-fill�.lev	"{Results}"
		Move -y R-goal.lev "{Results}"
		Move -y R-locate.lev "{Results}"
		Move -y R-start.lev "{Results}"

"{Results}"S-start.lev  � "{Dat}"Samurai.des LevComp
		LevComp "{Dat}"Samurai.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y S-fill�.lev	"{Results}"
		Move -y S-goal.lev "{Results}"
		Move -y S-locate.lev "{Results}"
		Move -y S-start.lev "{Results}"

"{Results}"T-start.lev  � "{Dat}"Tourist.des LevComp
		LevComp "{Dat}"Tourist.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y T-fill�.lev	"{Results}"
		Move -y T-goal.lev "{Results}"
		Move -y T-locate.lev "{Results}"
		Move -y T-start.lev "{Results}"

"{Results}"V-start.lev  � "{Dat}"Valkyrie.des LevComp
		LevComp "{Dat}"Valkyrie.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y V-fill�.lev	"{Results}"
		Move -y V-goal.lev "{Results}"
		Move -y V-locate.lev "{Results}"
		Move -y V-start.lev "{Results}"

"{Results}"W-start.lev  � "{Dat}"Wizard.des LevComp
		LevComp "{Dat}"Wizard.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y W-fill�.lev	"{Results}"
		Move -y W-goal.lev "{Results}"
		Move -y W-locate.lev "{Results}"
		Move -y W-start.lev "{Results}"

"{Results}"Quest.Dat	� "{Dat}"Quest.txt MakeDefs
		MakeDefs -q
		Move -y "{Dat}"Quest.Dat "{Results}"
		SetFile -t "{FileType}" -c "{FileCreator}" "{Results}"Quest.dat

"{Results}"Valley.lev  � "{Dat}"Gehennom.des LevComp
		LevComp "{Dat}"Gehennom.des
		SetFile -t "{FileType}" -c "{FileCreator}" �.lev
		Move -y Asmodeus.lev "{Results}"
		Move -y Baalz.lev "{Results}"
		Move -y Juiblex.lev "{Results}"
		Move -y Orcus.lev "{Results}"
		Move -y Sanctum.lev "{Results}"
		Move -y Valley.lev "{Results}"

"{Results}"NetHack  �� "{MacDir}"NetHack.r "{MacDir}"NetHack.rsrc {BuiltInData} "{MacDir}"Sounds.rsrc
		Set Dat "{Dat}"			# Make the internal Make variable a Shell variable
		Export Dat				# and export it so Rez can use it to find things
		Set ObjDir "{ObjDir}"
		Export ObjDir
		Rez "{MacDir}"NetHack.r  -s "{MacDir}" -append -o "{Results}"NetHack

"{Results}"NetHack	�� {CObjs}
		Link {NetHackLink} {CObjs} -o "{Results}"NetHack
   		Setfile -a B "{Results}"NetHack
		Beep C,12 A,12 C,12 B,12 C,15
