print join("\n", &MacPerl'Volumes()), "\n---\n";
$boot = &MacPerl'Volumes();
print $boot, "\n";
print &MacPerl'MakePath($boot), "\n";
print &MacPerl'MakeFSSpec("MacPerl"), "\n";
