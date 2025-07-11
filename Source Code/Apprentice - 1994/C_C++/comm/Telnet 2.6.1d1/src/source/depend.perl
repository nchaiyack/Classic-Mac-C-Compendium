#
# depend.perl
#
# Copyright 1993,
# Rick Watson
# The University of Texas at Austin
# Networking Services
# r.watson@utexas.edu
#
# usage: perl depend.perl <makefile> <cpp directory> <obj directory> <files...>
#

#
# Here's a partial example of usage:
# Make sure you have created cpp and obj directories.
#
#	CPP = �
#		:cpp:debug.cpp �
#		:cpp:sysheaders.cpp �
#		:cpp:configure.cpp �
#		:cpp:prefs.cpp �
#		:cpp:Sets.cpp �
#		:cpp:binsubs.cpp
#	
#	# default rule for relating object files to source files
#	:cpp: � �
#		: �
#		:config: �
#		:ftp: �
#		:init: �
#		:main: �
#		:network: �
#		:network:mactcp: �
#		:tek: �
#		:vr: �
#		:vs:
#	
#	# Rule to build .cpp preprocessor output files. Syntax check only and write cpp output.
#	.cpp � .c
#		C "{DepDir}{Default}.c" -e2 -c {COptions} > "{TargDir}{Default}.cpp"
#		
#	depend � {CPP}
#		perl {mpw}local:depend.perl Makefile ":cpp:" ":obj:" {CPP}
#		Rename -y Makefile Makefile.bak
#		Rename -y Makefile.new Makefile
#		Echo "Include �"Makefile.bak�" 'ckid';" | Rez -m -a -o "Makefile" #Transfer the ckid
#	

#
# Copy Makefile up to the dependancies line
#
if ($#ARGV < 4) {
	die "depend: not enough arguments\n";
}
$makefile = $ARGV[0];
$cppdir = $ARGV[1];
$objdir = $ARGV[2];

open (INPUT, "$makefile") || die "Could not open $makefile\n";
open (OUTPUT, ">$makefile.new") || die "Could not create $makefile.new\n";
while (<INPUT>) {
	if (/^# DO NOT DELETE THIS LINE -- mkdep uses it./) {
		last;
	}
	printf OUTPUT "%s", $_;
}
close (INPUT);

print OUTPUT "# DO NOT DELETE THIS LINE -- mkdep uses it.\n";
print OUTPUT "# DO NOT PUT ANYTHING AFTER THIS LINE. IT WILL GO AWAY.\n";
print OUTPUT "\n";

#
# process input files
#
for ($ii = 3; $ii <= $#ARGV; $ii++) {
	do depend($ARGV[$ii]);
}

print OUTPUT "\n";
print OUTPUT "# IF YOU PUT ANYTHING HERE IT WILL GO AWAY\n";
print OUTPUT "\n";
close (OUTPUT);

#
# depend a file
#
# The hardwired paths of :cpp: and :objs: are kinf of a hack. 
# Should try to improve this.
#
sub depend {

	undef(%includes);
	
	$cppfilename = $_[0];
	$filename = substr("$cppfilename", 0, length("$cppfilename")-4); # remove .cpp
	$filename = substr("$filename", length($cppdir)); # remove :cpp:
	
	open (FILE, ":cpp:$cppfilename") || die "Can't open input file :cpp:$cppfilename\n";
	
	while (<FILE>) {
		#
		# search for file references in the preprocessor output
		#
		if (/^#line \d* \"(.*)\"/) {
			$includes{$1} = 1;
			next;
		}
		if (/^include \"(.*)\"/) {
			$includes{$1} = 1;
			next;
		}
		if (/^#pragma load \"(.*)\"/) {
			$includes{$1} = 1;
			next;
		}
	}
	close (FILE);
	
	#
	# enumerate the references accumulated in $includes
	#
	$nf = 0;
	while (($key,$val) = each %includes) {
		$_ = $key;
		if (!(/MPW:Interfaces:CIncludes/)) {
			print OUTPUT "$objdir$filename.c.o � \"$key\"\n";
			$nf++;
		}
	}
	if ($nf == 0) {
		print "No dependancies found in $filename\n";
	} else {
		print OUTPUT "\n";
	}
}

