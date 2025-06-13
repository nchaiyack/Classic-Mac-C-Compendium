require "StandardFile.pl";
require "FileCopy.pl";

($from = &StandardFile'GetFile("From:", "TEXT")) &&
($to   = &StandardFile'PutFile("To:", "$from Copy")) &&
print &MacPerl'FileCopy($from, $to, 1);
