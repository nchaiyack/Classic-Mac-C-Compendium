package MacPerl;

&MacPerl'LoadExternals("FindFolder.pl");

#
# &MacPerl'FindFolder($foldertype [, $create])
# 
# Where $type is one of:
#
# A[pple menu]
# C[ontrol Panel]
# D[esktop]
# E[xtension]
# F[onts]
# P[references]
# M[onitor Printing]
# N[etwork trash]
# T[rash]
# I[tems startup]
# S[ystem]
# Y[TemporarY Items]
#
# If "true", $create specifies that the folder should be created if it
# doesn't exist yet.
#

1;