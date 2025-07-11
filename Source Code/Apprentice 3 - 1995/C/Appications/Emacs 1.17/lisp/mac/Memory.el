;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

;#pragma parameter __A0 NewPtr(__D0)
;pascal Ptr NewPtr(Size byteCount)
;    = 0xA11E; 
(deftrap NewPtr ("a11e")
  ((byteCount long d0))
  (long a0)
  t)

; #pragma parameter __A0 NewPtrClear(__D0)
; pascal Ptr NewPtrClear(Size byteCount)
;    = 0xA31E; 
(deftrap NewPtrClear ("a31e")
  ((byteCount long d0))
  (long a0)
  t)

;#pragma parameter __A0 NewHandle(__D0)
;pascal Handle NewHandle(Size byteCount)
;    = 0xA122; 
(deftrap NewHandle ("a122")
  ((byteCount long d0))
  (long a0)
  t)

; #pragma parameter __A0 NewHandleClear(__D0)
; pascal Handle NewHandleClear(Size byteCount)
;    = 0xA322; 
(deftrap NewHandleClear ("a322")
  ((byteCount long d0))
  (long a0)
  t)

;#pragma parameter DisposPtr(__A0)
;pascal void DisposPtr(Ptr p)
;    = 0xA01F; 
(deftrap DisposPtr ("a01f")
  ((p long a0))
  nil
  t)

;#pragma parameter DisposHandle(__A0)
;pascal void DisposHandle(Handle h)
;    = 0xA023; 
(deftrap DisposHandle ("a023")
  ((h long a0))
  nil
  t)

;#pragma parameter HLock(__A0)
;pascal void HLock(Handle h)
;    = 0xA029; 
(deftrap HLock ("a029")
  ((h long a0))
  nil
  t)

; #pragma parameter HUnlock(__A0)
; pascal void HUnlock(Handle h)
;    = 0xA02A; 
(deftrap HUnlock ("a02a")
  ((h long a0))
  nil
  t)

; pascal Size GetHandleSize(Handle h); 
(deftrap GetHandleSize ("a025")
  ((h long a0))
  (long d0)
  t)

; #pragma parameter SetHandleSize(__A0,__D0)
; pascal void SetHandleSize(Handle h,Size newSize)
;    = 0xA024; 
(deftrap SetHandleSize ("a024")
  ((h long a0)
   (newSize long d0))
  nil
  t)

; The incorrect version
;(deftrap MemError ("3038" "0220") ; move.w 0x220,d0
;  nil
;  (short d0))

; The correct version
(defun MemError ()
  elisp-MemError)

; #pragma parameter __D0 HGetState(__A0)
; pascal char HGetState(Handle h)
;     = 0xA069; 
(deftrap HGetState ("a069")
  ((h long a0))
  (char d0)
  t)

; #pragma parameter HSetState(__A0,__D0)
; pascal void HSetState(Handle h,char flags)
;     = 0xA06A; 
(deftrap HSetState ("a06a")
  ((h long a0)
   (flags char d0))
  nil
  t)
