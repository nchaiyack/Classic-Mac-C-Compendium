ND_OF_FILE
f test 0 -ne `wc -c <'test/error.tab.h'`; then
   echo shar: \"'test/error.tab.h'\" unpacked with wrong size!
i
 end of 'test/error.tab.h'
i
f test -f 'test/error.y' -a "${1}" != "-c" ; then 
 echo shar: Will not clobber existing file \"'test/error.y'\"
lse
cho shar: Extracting \"'test/error.y'\" \(117 characters\)
ed "s/^X//" >'test/error.y' <<'END_OF_FILE'
%%
S: error
%%
main(){printf("yyparse() = %d\n",yyparse());}
yylex(){return-1;}
yyerror(s)char*s;{printf("%s\n",s);}
