#
# Makefile for NCSA Telnet 2.6
#
# Rick Watson
# The University of Texas at Austin
# Networking Services
# r.watson@utexas.edu
#
# 9/29/93
#

#
# This makefile is only for source maintenance functions.
# See :source:Makefile to build Telnet with MPW
#

SRCS= ¶
	:Makefile ¶
	:--Telnet.rsrc ¶
	':myMac #includes.c' ¶
	:source:debug.c ¶
	:source:debug.h ¶
	:source:globaldefs.h ¶
	:source:mpw.h ¶
	:source:newresrc.h ¶
	:source:resrcdefs.h ¶
	:source:TelnetHeader.h ¶
	:source:typedefs.h ¶
	:source:Makefile ¶
	:source:config:configure.c ¶
	:source:config:configure.proto.h ¶
	:source:config:Preferences.h ¶
	:source:config:prefs.c ¶
	:source:config:prefs.proto.h ¶
	:source:config:Sets.c ¶
	:source:config:Sets.proto.h ¶
	:source:ftp:binsubs.c ¶
	:source:ftp:binsubs.proto.h ¶
	:source:ftp:bkgr.c ¶
	:source:ftp:bkgr.proto.h ¶
	:source:ftp:ftpbin.c ¶
	:source:ftp:ftpbin.proto.h ¶
	:source:ftp:ftppi.h ¶
	:source:ftp:macbinary.h ¶
	:source:ftp:macutil.c ¶
	:source:ftp:macutil.proto.h ¶
	:source:ftp:rcp.c ¶
	:source:init:iNetwork.c ¶
	:source:init:iNetwork.proto.h ¶
	:source:init:init.c ¶
	:source:init:init.proto.h ¶
	:source:main:ae.c ¶
	:source:main:Connections.c ¶
	:source:main:Connections.proto.h ¶
	:source:main:DlogUtils.c ¶
	:source:main:DlogUtils.proto.h ¶
	:source:main:errors.c ¶
	:source:main:telneterrors.h ¶
	:source:main:errors.proto.h ¶
	:source:main:event.c ¶
	:source:main:event.proto.h ¶
	:source:main:maclook.c ¶
	:source:main:maclook.proto.h ¶
	:source:main:macros.c ¶
	:source:main:macros.proto.h ¶
	:source:main:mainseg.c ¶
	:source:main:mainseg.proto.h ¶
	:source:main:menuseg.c ¶
	:source:main:menuseg.proto.h ¶
	:source:main:optsdebug.h ¶
	:source:main:parse.c ¶
	:source:main:parse.h ¶
	:source:main:parse.proto.h ¶
	:source:main:popup.c ¶
	:source:main:Popup.h ¶
	:source:main:popup.proto.h ¶
	:source:main:printing.c ¶
	:source:main:printing.proto.h ¶
	:source:main:translate.c ¶
	:source:main:translate.proto.h ¶
	:source:network:finger.c ¶
	:source:network:finger.proto.h ¶
	:source:network:fingerstructure.h ¶
	:source:network:InternalEvents.h ¶
	:source:network:mydnr.c ¶
	:source:network:mydnr.proto.h ¶
	:source:network:netevent.c ¶
	:source:network:netevent.proto.h ¶
	:source:network:MacTCP:AddressXlation.h ¶
	:source:network:MacTCP:dnr.c ¶
	:source:network:MacTCP:dnr.proto.h ¶
	:source:network:MacTCP:GetMyIPAddr.h ¶
	:source:network:MacTCP:MacTCPCommonTypes.h ¶
	:source:network:MacTCP:MyMacTCPstructures.h ¶
	:source:network:MacTCP:network.c ¶
	:source:network:MacTCP:network.proto.h ¶
	:source:network:MacTCP:TCPPB.h ¶
	:source:tek:rg0.c ¶
	:source:tek:rg0.proto.h ¶
	:source:tek:rghp.c ¶
	:source:tek:rghp.proto.h ¶
	:source:tek:rgmp.c ¶
	:source:tek:rgmp.proto.h ¶
	:source:tek:tekdefs.h ¶
	:source:tek:tekrgmac.c ¶
	:source:tek:tekrgmac.proto.h ¶
	:source:tek:tekstor.c ¶
	:source:tek:tekstor.proto.h ¶
	:source:tek:vgfont.h ¶
	:source:tek:vgtek.c ¶
	:source:tek:vgtek.proto.h ¶
	:source:vr:vdevice.c ¶
	:source:vr:vdevice.h ¶
	:source:vr:vdevice.proto.h ¶
	:source:vr:vr.c ¶
	:source:vr:vr.h ¶
	:source:vr:vr.proto.h ¶
	:source:vr:vrrgmac.c ¶
	:source:vr:vrrgmac.proto.h ¶
	:source:vs:rsdefs.h ¶
	:source:vs:rsmac.c ¶
	:source:vs:rsmac.proto.h ¶
	:source:vs:vsdata.h ¶
	:source:vs:vsem.c ¶
	:source:vs:vsem.proto.h ¶
	:source:vs:vsinit.h ¶
	:source:vs:vsinterf.c ¶
	:source:vs:vsinterf.proto.h ¶
	:source:vs:vsintern.c ¶
	:source:vs:vsintern.proto.h ¶
	:source:vs:vskeys.h ¶
	:source:vs:wind.h ¶
	:source:main:wdefpatch.c ¶
	:source:main:wdefpatch.proto.h ¶
	:source:sysheaders.c

#
# orphans
#
orphans Ä
	for i in {SRCS}
		Echo "Delete 'ckid';" | Rez -a -m -o "{i}"
	end

#
# newversion
#
# version 1.0 is the original 2.6b10
# version 2.0 is the MPW changes
# version 8.0 is K8 10/08/93
# version 9.0 is K9 10/14/93
# version 9.1 is misc changes 10/18/93, version sent to NCSA.

versionNumber= 9.1
checkopts= -y -m -t "V{versionNumber}"

newversion Ä
	for i in {SRCS}
		if "{i}" =~ /(?*:)¨1(?+)¨2/ # break into path(1) and filename(2)
			if "{¨1}" == ":"
				set pjct "telnet"
			else if "{¨1}" == ":source:"
				set pjct "telnetºsource"
			else if "{¨1}" == ":source:network:MacTCP:"
				set pjct "telnetºmactcp"
			else if "{¨1}" =~ /:(?+)¨3:(?+)¨4:/
				set pjct "telnetº{¨4}"
			else
				echo "Error in {¨1}"
			end
			echo checkin {checkopts} -project "{pjct}" "{i},{versionNumber}" ·· Dev:StdOut
			checkin {checkopts} -project "{pjct}" "{i},{versionNumber}" ·· Dev:StdOut
		end
	end  ³ Dev:Null # cmd output is pitched
	
#
# diffs
#
diffBase= 2.0

diffs Ä
	set echo 0
	export echo
	for i in {SRCS}
		rcsdiff -r {diffBase} {i} >> telnet.k.diffs ·· Dev:StdOut
	end ³ Dev:Null # cmd output is pitched
