#!./perl

$^I = '.bak';

# $Header: inplace.t,v 4.0 91/03/20 01:50:59 lwall Locked $

print "1..2\n";

@ARGV = ('.a','.b','.c');
`echo foo >.a`;
`echo foo >.b`;
`echo foo >.c`;
while (<>) {
    s/foo/bar/;
}
continue {
    print;
}

if (`catenate .a .b .c` eq "bar\nbar\nbar\n") {print "ok 1\n";} else {print "not ok 1\n";}
if (`catenate .a.bak .b.bak .c.bak` eq "foo\nfoo\nfoo\n") {print "ok 2\n";} else {print "not ok 2\n";}

unlink '.a', '.b', '.c', '.a.bak', '.b.bak', '.c.bak';
