#!./perl -P

# $RCSfile: cpp.t,v $$Revision: 4.0.1.1 $$Date: 92/06/08 15:42:08 $

print "1..3\n";

#this is a comment
#define MESS "ok 1\n"
print MESS;

#If you capitalize, it's a comment.
#ifdef MESS
	print "ok 2\n";
#else
	print "not ok 2\n";
#endif

open(TRY,">Comp.cpp.tmp") || die "Can't open temp perl file.";

($prog = <<'END') =~ s/X/#/g;
$ok = "not ok 3\n";
Xinclude "Comp.cpp.inc"
Xifdef OK
$ok = OK;
Xendif
print $ok;
END
print TRY $prog;
close TRY;

open(TRY,">Comp.cpp.inc") || (die "Can't open temp include file.");
print TRY '#define OK "ok 3\n"' . "\n";
close TRY;

$pwd=`directory`;
$pwd =~ s/\n//;
$x = `perl -P Comp.cpp.tmp`;
print $x;
unlink "Comp.cpp.tmp", "Comp.cpp.inc";
