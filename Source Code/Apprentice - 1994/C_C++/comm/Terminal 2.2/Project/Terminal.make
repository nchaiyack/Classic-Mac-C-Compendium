# Terminal 2.0
# MPW 3.2 make file

OBJ 		= :(Objects):	# Objects folder
SRC			= :Sources:		# Sources folder

# All application header files

HEADERS		= �
			{SRC}CancelDialog.h �
			{SRC}CisB.h �
			{SRC}CRC.h �
			{SRC}Document.h �
			{SRC}File.h �
			{SRC}FormatStr.h �
			{SRC}Interp.h �
			{SRC}MacBinary.h �
			{SRC}Macros.h �
			{SRC}Main.h �
			{SRC}MySF.h �
			{SRC}Options.h �
			{SRC}Popup.h �
			{SRC}Procedure.h �
			{SRC}Scroll.h �
			{SRC}Serial.h �
			{SRC}Strings.h �
			{SRC}Text.h �
			{SRC}Utilities.h �
			{SRC}XModem.h �
			{SRC}ZModem.h

# All object files

OBJECTS		= �
			{OBJ}About.c.o �
			{OBJ}CancelDialog.c.o �
			{OBJ}CisB.c.o �
			{OBJ}CRC.c.o �
			{OBJ}Document.c.o �
			{OBJ}File.c.o �
			{OBJ}FormatStr.c.o �
			{OBJ}Glue.a.o �
			{OBJ}Init.c.o �
			{OBJ}Interp.c.o �
			{OBJ}MacBinary.c.o �
			{OBJ}Macros.c.o �
			{OBJ}Main.c.o �
			{OBJ}MySF.c.o �
			{OBJ}OtherOptions.c.o �
			{OBJ}Popup.c.o �
			{OBJ}PortOptions.c.o �
			{OBJ}Procedure.c.o �
			{OBJ}ProtocolOptions.c.o �
			{OBJ}Script.c.o �
			{OBJ}Scroll.c.o �
			{OBJ}SendOptions.c.o �
			{OBJ}Serial.c.o �
			{OBJ}TerminalOptions.c.o �
			{OBJ}Text.c.o �
			{OBJ}Utilities.c.o �
			{OBJ}XModem.c.o �
			{OBJ}XOptions.c.o �
			{OBJ}ZModem.c.o �
			{OBJ}ZOptions.c.o

# C options

OPT = -r -mbg off -sym full -o {OBJ}


Terminal	�� {OBJECTS}
			Link	-sym full -x {OBJ}CrossRef �
					-ra Main=resLocked,resProtected,resPreload �
					-ra Main2=resLocked,resProtected,resPreload �
					-ra Init=resPurgeable,resProtected �
					-ra Options=resPurgeable,resProtected �
					-ra About=resPurgeable,resProtected �
					-ra CisB=resPurgeable,resProtected �
					-ra XModem=resPurgeable,resProtected �
					-ra ZModem=resPurgeable,resProtected �
					-sg Main=STDCLIB �
 					-o {Targ} {OBJECTS} �
					{CLibraries}StdCLib.o �
					{Libraries}Runtime.o �
					{Libraries}Interface.o
			SetFile {Targ} -t APPL -c 'ET_T' -a B


Terminal	�� 'Terminal �.Rsrc' {SRC}Terminal.r
			Rez -a -o {Targ} -i {RIncludes}Types.r {SRC}Terminal.r


{OBJ}MacHeadersMPW			� {SRC}MacHeadersMPW.c
							C {OPT} {SRC}MacHeadersMPW.c

{OBJ}About.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}About.c
							C {OPT} {SRC}About.c

{OBJ}CancelDialog.c.o		� {OBJ}MacHeadersMPW {HEADERS} {SRC}CancelDialog.c
							C {OPT} {SRC}CancelDialog.c

{OBJ}CisB.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}CisB.c
							C {OPT} {SRC}CisB.c

{OBJ}CRC.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}CRC.c
							C {OPT} {SRC}CRC.c

{OBJ}Document.c.o			� {OBJ}MacHeadersMPW {HEADERS} {SRC}Document.c
							C {OPT} {SRC}Document.c

{OBJ}File.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}File.c
							C {OPT} {SRC}File.c

{OBJ}FormatStr.c.o			� {OBJ}MacHeadersMPW {HEADERS} {SRC}FormatStr.c
	 						C {OPT} {SRC}FormatStr.c

{OBJ}Glue.a.o				� {SRC}Glue.a
	 						Asm -o {OBJ} {SRC}Glue.a

{OBJ}Init.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Init.c
							C {OPT} {SRC}Init.c

{OBJ}Interp.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Interp.c
							C {OPT} {SRC}Interp.c

{OBJ}MacBinary.c.o			� {OBJ}MacHeadersMPW {HEADERS} {SRC}MacBinary.c
							C {OPT} {SRC}MacBinary.c

{OBJ}Macros.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Macros.c
							C {OPT} {SRC}Macros.c

{OBJ}Main.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Main.c
							C {OPT} {SRC}Main.c

{OBJ}MySF.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}MySF.c
							C {OPT} {SRC}MySF.c

{OBJ}OtherOptions.c.o		� {OBJ}MacHeadersMPW {HEADERS} {SRC}OtherOptions.c
							C {OPT} {SRC}OtherOptions.c

{OBJ}Popup.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Popup.c
							C {OPT} {SRC}Popup.c

{OBJ}PortOptions.c.o		� {OBJ}MacHeadersMPW {HEADERS} {SRC}PortOptions.c
							C {OPT} {SRC}PortOptions.c

{OBJ}Procedure.c.o			� {OBJ}MacHeadersMPW {HEADERS} {SRC}Procedure.c
							C {OPT} {SRC}Procedure.c

{OBJ}ProtocolOptions.c.o	� {OBJ}MacHeadersMPW {HEADERS} {SRC}ProtocolOptions.c
							C {OPT} {SRC}ProtocolOptions.c

{OBJ}Script.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Script.c
							C {OPT} {SRC}Script.c

{OBJ}Scroll.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Scroll.c
							C {OPT} {SRC}Scroll.c

{OBJ}SendOptions.c.o		� {OBJ}MacHeadersMPW {HEADERS} {SRC}SendOptions.c
							C {OPT} {SRC}SendOptions.c

{OBJ}Serial.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Serial.c
							C {OPT} {SRC}Serial.c

{OBJ}TerminalOptions.c.o	� {OBJ}MacHeadersMPW {HEADERS} {SRC}TerminalOptions.c
							C {OPT} {SRC}TerminalOptions.c

{OBJ}Text.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}Text.c
							C {OPT} {SRC}Text.c

{OBJ}Utilities.c.o			� {OBJ}MacHeadersMPW {HEADERS} {SRC}Utilities.c
							C {OPT} {SRC}Utilities.c

{OBJ}XModem.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}XModem.c
							C {OPT} {SRC}XModem.c

{OBJ}XOptions.c.o			� {OBJ}MacHeadersMPW {HEADERS} {SRC}XOptions.c
							C {OPT} {SRC}XOptions.c

{OBJ}ZModem.c.o				� {OBJ}MacHeadersMPW {HEADERS} {SRC}ZModem.c
							C {OPT} {SRC}ZModem.c

{OBJ}ZOptions.c.o			� {OBJ}MacHeadersMPW {HEADERS} {SRC}ZOptions.c
							C {OPT} {SRC}ZOptions.c
