;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

;extern pascal OSErr AEResolve(const AEDesc *objectSpecifier, short callbackFlags, AEDesc *theToken)
; THREEWORDINLINE(0x303C, 0x0536, 0xA816);
(deftrap AEResolve ("303c" "0536" "a816")
  ((objectSpecifier address)
   (callbackFlags short)
   (theToken address))
  short)
 
;extern pascal OSErr AEInstallObjectAccessor(DescType desiredClass, DescType containerType,
;OSLAccessorUPP theAccessor, long accessorRefCon, Boolean isSysHandler)
; THREEWORDINLINE(0x303C, 0x0937, 0xA816);
(deftrap AEInstallObjectAccessor-internal ("303c" "0937" "a816")
  ((desiredClass immediate-string)
   (containerType immediate-string)
   (theAccessor address)
   (accessorRefCon address)
   (isSysHandler short))
  short)

;;; This list is only used to protect the cons cells stored in the
;;; refCon slots of the dispatch table from being garbage collected, and for
;;; documentation.
(defvar AEObjAcc-callback-list nil "The list of Apple event object accessors")

(defun AEObjAcc-callback-function (desiredClass containerToken containerClass keyForm keyData
												token theRefCon)
  (DebugStr "In AEObjAcc-callback-function, " desiredClass containerClass)
  (funcall (car theRefCon) desiredClass containerToken
		   containerClass keyForm keyData token (cdr theRefCon)))

(defun AEInstallObjectAccessor (desiredClass containerType theAccessor accessorRefCon isSysHandler)
  (let* ((callback-cons (cons theAccessor accessorRefCon))
		 (err (AEInstallObjectAccessor-internal desiredClass containerType AEObjAcc-callback-function
												callback-cons isSysHandler)))
    (if (not (zerop err))
		err
      (setq AEObjAcc-callback-list (cons (list desiredClass containerType callback-cons)
										 AEObjAcc-callback-list))
      noErr)))
