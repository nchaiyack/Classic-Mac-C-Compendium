#!perl

require "FindFolder.pl";

for ($cmd = "ACDEFPMNTISY"; $c = chop $cmd; ) {
   print "$c: ", &MacPerl'FindFolder($c), "\n";
}
