ftpobj = "{obj}"ftp:

OBJS = "{obj}"unixlib.o �
       "{Libraries}"Interface.o �
       "{Libraries}"Runtime.o �
       "{CLibraries}"StdCLib.o �
       "{obj}"socket.o �
	   "{obj}"app.o

COptions = -d MAKE_FTP_COMPILE -d SIGPIPE=99 -i "{unixinc}" -i "{app}" -mbg full -sftp

MAP = -l -lf > map.dat
TOOL = -t MPST -c 'MPS '

all � "{bin}"ftp 

#########################################################################

FTP_OBJS =	"{ftpobj}"cmds.c.o �
            "{ftpobj}"cmdtab.c.o �
            "{ftpobj}"ftp.c.o �
            "{ftpobj}"glob.c.o �
            "{ftpobj}"main.c.o �
            "{ftpobj}"ruserpass.c.o �
            "{ftpobj}"domacro.c.o �
			{OBJS}

"{bin}"ftp � {FTP_OBJS}
	link -d {TOOL} {FTP_OBJS} -o "{bin}"ftp

"{ftpobj}"cmds.c.o      � ftp_var.h
"{ftpobj}"cmdtab.c.o    � ftp_var.h
"{ftpobj}"ftp.c.o       � ftp_var.h
"{ftpobj}"main.c.o      � ftp_var.h
"{ftpobj}"ruserpass.c.o � ftp_var.h
"{ftpobj}"domacro.c.o   � ftp_var.h

#########################################################################

"{ftpobj}" � "::ftp:"

.c.o	�	.c
	c "{depDir}{default}.c" -o "{targDir}{default}.c.o" {COptions}


