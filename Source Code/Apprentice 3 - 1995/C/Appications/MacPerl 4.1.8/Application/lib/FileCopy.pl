package FileCopy;

&MacPerl'LoadExternals("FileCopy.pl");

package MacPerl;

#
# &MacPerl'FileCopy($from, $to [, $replace [, $dontresolve]])
# 

sub FileCopy {
   local($from, $to, $replace, $dontresolve) = @_;

   if ($replace) {
      $replace = "true";
   } else {
      $replace = "false";
   }

   if ($dontresolve) {
      &FileCopy'FileCopy($from, $to, $replace, "DontResolveAlias", 
         "DontShowProgress");
   } else {
      &FileCopy'FileCopy($from, $to, $replace, "DontShowProgress");
   }
}

1;