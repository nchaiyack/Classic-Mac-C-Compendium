# Makefile for any machine
# - based on my newer makefile and the GREATE
# modifications by Mike Battersby
###############################################################################
#
# CONFIGURATION OPTIONS
#

# Choose your machine type. Possible types as of this version (4.0)
# are
#  hpalib, hplow, hp3, dec, solaris, sparc, linux, freebsd, sgi, soundblaster.
MACHINE = sparc

# The name of your C compiler. For most machines this will be
# either 'cc' or 'gcc'.
CC = cc

# The suffix for object files. This will usually be .o, except for sgi (.j)
O = .o
OPTS=-c
# O =.j
# OPTS=-j

# User interface type. Right now, this is Unix as there are no alternatives.
UI = Unix/./

# Destination directory for tracker binaries and manpage.
#
# If you don't wish to use the 'make install' and 'make install.man'
# commands, you can ignore these.
BIN_DIR = /usr/local/bin
MAN_DIR = /usr/local/man

# How to install the binaries and manpage. If you have a unix system,
# use the GNU install program if you have it, otherwise try 'cp'. For
# non-unix systems, make this the command for copying a file with the
# following syntax
#  <program_name> <source_name> <destination_name>
#
# If you don't wish to use the 'make install' and 'make install.man'
# commands, you can ignore this.
INSTALL = install

# Permissions and user/group id's to install with. Non-unix users
# should comment these out (alternatively, you can use these to
# provide arguments for the above $(INSTALL) program). If you want
# to install tracker setuid root (which will allow it to renice
# itself when non-root users run it), change the "-m 755" below to
# "-m 4755". If you do so, you must be root to run make install.
#
# If you don't wish to use the 'make install' and 'make install.man'
# commands, you can ignore these.
INST_BIN_OPT = -s -m 755 -o root -g root
INST_MAN_OPT = -m 644 -o root -g root
###############################################################################
#
# C compiler flags and libraries for each machine.
# Unless you are porting this to a new architecture, you shouldn't
# need to change these (on the other hand, if you understand what you
# are doing, go ahead!).
#

CFLAGS_hpalib = +O3 +OS +ESlit
LIBS_hpalib   = -lAt -lAlib -lm
AUDIO_hpalib = Hpux/alib_
CONFIG_hpalib = Hpux/

CFLAGS_hplow = -Ae +O3 -Wl, -a,archive -s
LIBS_hplow=-lM
AUDIO_hplow = Hpux/low_
CONFIG_hplow = Hpux/

# yet another port to hp! This one might be the best yet.
CFLAGS_hp3 = +O3 +OS +ESlit
LIBS_hp3   = -lAt -lAlib -lm
AUDIO_hp3 = Hpux/3_
CONFIG_hp3 = Hpux/3_

CFLAGS_solaris = -O
LIBS_solaris   = -lm
AUDIO_solaris = Sparc/./
CONFIG_solaris = Sparc/solaris_


CFLAGS_sparc = -g -Bstatic
LIBS_sparc   = -lm
AUDIO_sparc = Sparc/./
CONFIG_sparc = Sparc/

CFLAGS_dec = -O 
LIBS_dec   = -lm -lAF
AUDIO_dec = AF/
CONFIG_dec = AF/dec_

CFLAGS_sgi = -j -O 
LIBS_sgi   = -laudio -lm
AUDIO_sgi = Sgi/
CONFIG_sgi = Sgi/

CFLAGS_soundblaster = -O
LIBS_soundblaster   = -lm
AUDIO_soundblaster = Soundblaster/
CONFIG_soundblaster = Soundblaster/

CFLAGS_linux = -N -O2 -funroll-loops
LIBS_linux   = -lm
AUDIO_linux = Linux/./
CONFIG_linux = Linux/

CFLAGS_freebsd = -O6 -funroll-loops
LIBS_freebsd = -lm
AUDIO_freebsd = Linux/
CONFIG_freebsd = Linux/

CFLAGS = $(CFLAGS_$(MACHINE))
COPTS = $(OPTS) $(CFLAGS)
LIBS = $(LIBS_$(MACHINE))
CONFIG = $(CONFIG_$(MACHINE))
AUDIO = $(AUDIO_$(MACHINE))

OBJ_TRACKER = main$O $(AUDIO)audio$O st_read$O commands$O \
audio$O automaton$O player$O getopt$O open$O tools$O \
dump_song$O setup_audio$O notes$O display$O \
$(UI)ui$O prefs$O tags$O autoinit$O

OBJ_ANALYZER = analyzer$O st_read$O open$O dump_song$O tools$O notes$O \
prefs$O autoinit$O ${UI}ui$O display$O

OBJ_SPLITMOD = split$O tools$O st_read$O dump_song$O open$O notes$O \
display$O prefs$O autoinit$O $(UI)ui$O


all:	config.h tracker randomize analyzer splitmod

config.h:
	cp $(CONFIG)config.h config.h

install: 
	$(INSTALL) $(INST_BIN_OPT) tracker $(BIN_DIR)
	$(INSTALL) $(INST_BIN_OPT) randomize $(BIN_DIR)
	$(INSTALL) $(INST_BIN_OPT) analyzer $(BIN_DIR)

install.man:
	$(INSTALL) $(INST_MAN_OPT) docs/tracker.1 $(MAN_DIR)/man1/tracker.1



devel: 
	-rm -f $(INSTALL_DIR)/dtracker $(INSTALL_DIR)/splitmod
	cp tracker $(INSTALL_DIR)/dtracker
	cp splitmod $(INSTALL_DIR)/splitmod

tracker: ${OBJ_TRACKER}
	${CC} -o tracker ${CFLAGS} ${OBJ_TRACKER} ${LIBS}

analyzer: ${OBJ_ANALYZER}
	$(CC) -o analyzer ${CFLAGS} ${OBJ_ANALYZER} ${LIBS}

randomize: randomize.c
	$(CC) -o randomize ${CFLAGS} randomize.c 

splitmod: ${OBJ_SPLITMOD}
	$(CC) -o splitmod ${CFLAGS} ${OBJ_SPLITMOD} ${LIBS}

main$O: main.c song.h
	$(CC) ${COPTS} main.c

tools$O: main.c 
	$(CC) ${COPTS} tools.c

$(AUDIO)audio$O: $(AUDIO)audio.c 
	cd `dirname $(AUDIO)`; $(CC) -I.. ${COPTS} `basename $(AUDIO)`audio.c

$(UI)ui$O: $(UI)ui.c
	cd $(UI); $(CC) -I.. ${COPTS} ui.c

split$O: split.c
	$(CC) ${COPTS} split.c

open$O: open.c
	$(CC) ${COPTS} open.c

audio$O: audio.c
	$(CC) ${COPTS} audio.c

automaton$O: automaton.c song.h channel.h
	$(CC) ${COPTS} automaton.c

getopt$O: getopt.c getopt.h
	$(CC) ${COPTS} getopt.c

getopt1$O: getopt1.c getopt.h
	$(CC) ${COPTS} getopt1.c

player$O: player.c song.h channel.h
	$(CC) ${COPTS} player.c

st_read$O: st_read.c song.h
	$(CC) ${COPTS} st_read.c

commands$O: commands.c channel.h song.h
	$(CC) ${COPTS} commands.c

unix/termio$O: unix/termio.c
	$(CC) ${COPTS} unix/termio.c
dump_song$O: dump_song.c
	$(CC) $(COPTS) dump_song.c
analyzer$O: analyzer.c
	$(CC) $(COPTS) analyzer.c
setup_audio$O: setup_audio.c
	$(CC) $(COPTS) setup_audio.c
notes$O: notes.c
	$(CC) $(COPTS) notes.c

display$O: display.c
	$(CC) $(COPTS) display.c


clean:
	-rm $(OBJ_TRACKER) $(OBJ_ANALYZER) $(OBJ_SPLITMOD) randomize$O
	-rm tracker randomize splitmod analyzer core 
	-rm tracker.tar tracker.tar.Z tracker.lzh

export:
	-rm tracker.tar.Z
	-rm tracker.lzh
	cd ..; lha a tracker/tracker.lzh tracker/*.[ch] \
	tracker/*/*.[ch] tracker/*/Docs/* \
	tracker/Amiga/server/*.[ch] \
	tracker/man/* \
	tracker/*akefile tracker/Docs/*
	cd ..; tar cvf tracker/tracker.tar tracker/*.[ch] \
	tracker/*/*.[ch] tracker/*/*/*.[ch] \
	tracker/*/Docs/* \ tracker/man/* tracker/Docs/* tracker/*akefile
	compress tracker.tar
