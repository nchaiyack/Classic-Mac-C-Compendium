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
#

SRCS= �
	:desproto.h �
	:enc_des.c �
	:encrypt.c �
	:encrypt.h �
	:glue.h �
	:kerberos.c �
	:KrbDriver.h �
	:Makefile �
	:des_cornell.c �
	:kerberos.h 

#
# orphans
#
orphans �
	for i in {SRCS}
		Echo "Delete 'ckid';" | Rez -a -m -o "{i}"
	end

#
# newversion
#
versionNumber= 11.0
checkopts= -y -m -t "V{versionNumber}"

newversion �
	for i in {SRCS}
		if "{i}" =~ /(?*:)�1(?+)�2/ # break into path(1) and filename(2)
			if "{�1}" == ":"
				set pjct "krb"
			else
				echo "Error in {�1}"
			end
			echo checkin {checkopts} -project "{pjct}" "{i},{versionNumber}" �� Dev:StdOut
			checkin {checkopts} -project "{pjct}" "{i},{versionNumber}" �� Dev:StdOut
		end
	end  � Dev:Null # cmd output is pitched
	
#
# diffs
#
diffBase= 9.0

diffs �
	set echo 0
	export echo
	for i in {SRCS}
		rcsdiff -r {diffBase} {i} >> telnet.diffs �� Dev:StdOut
	end � Dev:Null # cmd output is pitched
