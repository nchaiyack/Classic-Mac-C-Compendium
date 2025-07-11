#!perl

require "FAccess.ph";

print &MacPerl'FAccess("$ENV{MPW}Worksheet", &F_GTABINFO), "\n";
&MacPerl'FAccess("$ENV{MPW}Worksheet", &F_STABINFO, 7);
$res = &MacPerl'FAccess("$ENV{MPW}Worksheet", &F_GFONTINFO);
print "$res\n";
print join(", ", &MacPerl'FAccess("$ENV{MPW}Worksheet", &F_GFONTINFO)), "\n";
# &MacPerl'FAccess("$ENV{MPW}Worksheet", &F_SFONTINFO, "Chicago", 12);
&MacPerl'FAccess("$ENV{MPW}Worksheet", &F_SFONTINFO, "Geneva");
# &MacPerl'FAccess("$ENV{MPW}Worksheet", &F_SFONTINFO, 4, 9);
$res = &MacPerl'FAccess("$ENV{MPW}Worksheet", &F_GWININFO);
print "$res\n";
print join(", ", &MacPerl'FAccess("$ENV{MPW}Worksheet", &F_GWININFO)), "\n";
&MacPerl'FAccess("$ENV{MPW}Worksheet", &F_SWININFO, 200, 200, 400, 400);
$res = &MacPerl'FAccess("knuth:Development:Perl:config.h", &F_GSELINFO);
print "$res\n";
print join(", ", &MacPerl'FAccess("knuth:Development:Perl:config.h", &F_GSELINFO)), "\n";
&MacPerl'FAccess("knuth:Development:Perl:config.h", &F_SSELINFO, 1000, 1200);
