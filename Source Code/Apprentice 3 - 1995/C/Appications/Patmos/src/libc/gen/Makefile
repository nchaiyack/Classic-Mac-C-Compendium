CC=gcc
CFLAGS=-O

DEAD=clock.o disklabel.o fstab.o getlogin.o getmntinfo.o getpass.o nlist.o \
        setjmperr.o setmode.o siginterrupt.o signal.o sigsetops.o\
        isatty.o opendir.o syslog.o  termios.o times.o wait.o  wait3.o \
        popen.o ttyname.o regerror.o regexp.o regsub.o waitpid.o

SRCS = isatty.o \
alarm.o		errlst.o	isctype.o	scandir.o	timezone.o \
closedir.o	exec.o		nice.o		seekdir.o	ttyslot.o \
crypt.o		getcwd.o	pause.o		setrgid.o	ualarm.o \
ctermid.o	getsubopt.o	psignal.o	setruid.o	unvis.o \
ctime.o		getttyent.o	raise.o		sleep.o		usleep.o \
ctype_.o	getusershell.o	readdir.o	telldir.o	utime.o \
difftime.o	initgroups.o	rewinddir.o	time.o		valloc.o

all: $(SRCS)
	ar rs ../libc.a $(SRCS)
