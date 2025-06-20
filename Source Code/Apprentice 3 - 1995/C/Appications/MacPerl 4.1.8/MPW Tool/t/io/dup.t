#!./perl

# $Header: dup.t,v 4.0 91/03/20 01:50:49 lwall Locked $

print "1..6\n";

print "ok 1\n";

open(dupout,">&STDOUT");
open(duperr,">&STDERR");

open(STDOUT,">Io.dup") || die "Can't open stdout";
open(STDERR,">&STDOUT") || die "Can't open stderr";

select(STDERR); $| = 1;
select(STDOUT); $| = 1;

print STDOUT "ok 2\n";
print STDERR "ok 3\n";
print `echo ok 4`;
print STDERR `echo ok 5`;

close(STDOUT);
close(STDERR);

open(STDOUT,">&dupout");
open(STDERR,">&duperr");

print `catenate Io.dup`;
unlink 'Io.dup';

print STDOUT "ok 6\n";
