; pascal OSErr
; CreateObjSpecifier(	DescType desiredClass,
;						AEDesc *theContainer, 
;						DescType keyForm,
;						AEDesc *keyData, 
;						Boolean disposeInputs,
;						AEDesc *objSpecifier )

(if (not noninteractive)
	(deftrap CreateObjSpecifier ("4eb9"
								 (lsh CreateObjSpecifier -16)
								 (logand CreateObjSpecifier 65535))
	  ((desiredClass immediate-string)
	   (theContainer address)
	   (keyForm immediate-string)
	   (keyData address)
	   (disposeInputs char)
	   (objSpecifier address))
	  short))
