;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

;;;
;;; This generates the Toolbox trap glue.
;;;

(defun gencode (words offset)
  (if (null words)
      nil
    (cons
     (list 'encode-internal 'mac-trap-code offset ''short
		   (list 'hex-string-to-int (car words)))
     (gencode (cdr words) (+ (c:sizeof 'short) offset)))))

(defun munge-parameters (parameters)
  (if (null parameters)
      (list 0 nil nil nil)
    (let* ((parameter (car parameters))
		   (name (nth 0 parameter))
		   (size (nth 1 parameter))
		   (register (nth 2 parameter))
		   (cdr-munge (munge-parameters (cdr parameters)))
		   (offset (nth 0 cdr-munge))
		   (cdr-stack (nth 1 cdr-munge))
		   (cdr-registers (nth 2 cdr-munge))
		   (cdr-names (nth 3 cdr-munge))
		   (target (if (eq size 'address)
					   (` (let ((temp (, name)))
							(if (stringp temp) (string-data temp) temp)))
					 name))
		   (typeparams (assoc size '((char 2 'char)
									 (short 2 'short)
									 (long 4 'long)
									 (address 4 'verbatim-long)
									 (immediate-string 4 'string))))
		   (offset-incr (nth 1 typeparams))
		   (type-code (nth 2 typeparams)))
      (if register
		  (list
		   offset
		   cdr-stack
		   (` ((list (quote (, register)) (, target)) (,@ cdr-registers)))
		   (cons name cdr-names))
		(list
		 (+ offset offset-incr)
		 (cons (` (encode-internal stack (, offset) (, type-code) (, target))) cdr-stack)
		 cdr-registers
		 (cons name cdr-names))))))
	   
(defmacro deftrap (name code parameters return &optional save-MemError)
  (let* ((code-retrieve (cond
						 ((null return) nil)
						 ((consp return) nil)
						 ((eq return 'char) '("101f"))	  ; move.b (a7)+,d0
						 ((eq return 'short) '("301f"))	  ; move.w (a7)+,d0
						 ((eq return 'long) '("201f"))))  ; move.l (a7)+,d0
		 (code-rts (append code code-retrieve '("4e75"))) ; rts
		 (code-size (* 2 (length code-rts)))
		 (stack-result-size (cond ((null return) 0)
								  ((consp return) 0)
								  ((eq return 'char) 2)
								  ((eq return 'short) 2)
								  ((eq return 'long) 4)))
		 (munge-list (munge-parameters parameters))
		 (register-list (cons 'list (nth 2 munge-list)))
		 (stack-size (+ stack-result-size (nth 0 munge-list)))
		 (parameter-list (nth 3 munge-list))
		 (return-directive (cond ((null return) nil)
								 ((consp return) (list 'quote return))
								 ((eq return 'char) ''(char d0))
								 ((eq return 'short) ''(short d0))
								 ((eq return 'long) ''(long d0))))
		 (old-mac-trap-code-end mac-trap-code-end))
    (if (> (+ mac-trap-code-end code-size) mac-trap-code-max)
		(error "Out of code space for traps"))
    (setq mac-trap-code-end (+ mac-trap-code-end code-size))
    (` (progn
		 (if noninteractive
			 (message (concat "Defining trap " (, (symbol-name name)) "...")))
		 (,@ (gencode code-rts old-mac-trap-code-end))
		 (defun (, name) (, parameter-list)
		   (let ((stack (make-string (, stack-size) 0)))
			 (,@ (nth 1 munge-list))
			 (execute-68k-trap stack mac-trap-code (, old-mac-trap-code-end)
							   (, register-list) (, return-directive)
							   (, save-MemError))))))))

;;;
;;; Below are traps that haven't yet been properly classfied into the right files.
;;;

(deftrap DebugStr-internal ("abff")
  ((s address))
  nil)

(defun DebugStr (&rest s)
  (DebugStr-internal
   (CtoPstr
    (apply (function concat)
		   (mapcar (function (lambda (x)
							   (if (stringp x) x (prin1-to-string x))))
				   s)))))

;pascal OSErr NewAlias(const FSSpec *fromFile,
;		       const FSSpec *target,
;		       AliasHandle *alias)
;    = {0x7002,0xA823}; 
(deftrap NewAlias ("7002" "a823")
  ((fromFile address)
   (toFile address)
   (alias address))
  short)

;pascal OSErr NewAliasMinimal(const FSSpec *target,
;			      AliasHandle *alias)
;    = {0x7008,0xA823}; 
(deftrap NewAliasMinimal ("7008" "a823")
  ((target address)
   (alias address))
  short)

;pascal OSErr NewAliasMinimalFromFullPath(short fullPathLength,
;					  const unsigned char *fullPath,
;					  ConstStr32Param zoneName,
;					  ConstStr31Param serverName,
;					  AliasHandle *alias)
;    = {0x7009,0xA823}; 
(deftrap NewAliasMinimalFromFullPath ("7009" "a823")
  ((fullPathLength short)
   (fullPath address)
   (zoneName address)
   (serverName address)
   (alias address))
  short)

;#pragma parameter BlockMove(__A0,__A1,__D0)
;pascal void BlockMove(const void *srcPtr,void *destPtr,Size byteCount)
;    = 0xA02E; 
(deftrap BlockMove ("a02e")
  ((srcPtr address a0)
   (destPtr address a1)
   (byteCount long d0))
  nil)

; #pragma parameter GetDateTime(__A0)
; pascal void GetDateTime(unsigned long *secs)
;    = {0x20B8,0x020C}; 
(deftrap GetDateTime ("20b8" "020c")
  ((now address a0))
  nil)

;(defmacro GetDateTime (now)
;  (` (let ((s (make-string 4 0)))
;       (GetDateTime-internal s)
;       (setq (, now) (extract-internal s 0 'long)))))

; pascal void StandardPutFile(ConstStr255Param prompt,
;			     ConstStr255Param defaultName,
;			     StandardFileReply *reply)
;    = {0x3F3C,0x0005,0xA9EA}; 
(deftrap StandardPutFile ("3f3c" "0005" "a9ea")
  ((prompt address)
   (defaultName address)
   (reply address))
  nil)

; pascal OSErr FSpGetFInfo(const FSSpec *spec,FInfo *fndrInfo)
;  = {0x303C,0x0007,0xAA52}; 
(deftrap FSpGetFInfo ("303c" "0007" "aa52")
  ((spec address)
   (fndrInfo address))
  short)

(defconst sizeof-StandardFileReply 88)
(defun StandardFileReply-sfGood (s) (extract-internal s 0 'char))
(defun StandardFileReply-sfFile (s) (extract-internal s 6 'string (c:sizeof 'FSSpec)))
(defconst sizeof-SFTypeList 16)

(defun PutFile (prompt defaultName)
  (let ((reply (make-string sizeof-StandardFileReply 0)))
    (StandardPutFile (CtoPstr prompt) (CtoPstr defaultName) reply)
    (if (zerop (StandardFileReply-sfGood reply))
	nil
      (FSSpec-to-unix-filename (StandardFileReply-sfFile reply)))))

; pascal void StandardGetFile(FileFilterProcPtr fileFilter,
;			     short numTypes,
;			     SFTypeList typeList,
;			     StandardFileReply *reply)
;    = {0x3F3C,0x0006,0xA9EA}; 
(deftrap StandardGetFile ("3f3c" "0006" "a9ea")
  ((fileFilter address)
   (numTypes short)
   (typeList address)
   (reply address))
  nil)

(defun GetFile ()
  (let ((typeList (make-string sizeof-SFTypeList 0))
	(reply (make-string sizeof-StandardFileReply 0)))
    (encode-internal typeList 0 'string "TEXT")
    (StandardGetFile 0 1 typeList reply)
    (if (zerop (StandardFileReply-sfGood reply))
	nil
      (FSSpec-to-unix-filename (StandardFileReply-sfFile reply)))))

; #pragma parameter __D0 StripAddress(__D0)
; pascal Ptr StripAddress(void *theAddress)
;    = 0xA055; 
(deftrap StripAddress ("a055")
  ((theAddress address d0))
  (long d0))

; pascal PicHandle GetPicture(short pictureID)
;     = 0xA9BC; 
(deftrap GetPicture ("a9bc")
  ((pictureID short))
  long)

(deftrap a5 ("200d") ; move.l a5,d0
  nil
  (long d0))

(defun screenBits ()
  (- (deref (a5)) 122))

(deftrap TickCount ("2038" "016a") ; move.l 0x16a,d0
  nil
  (long d0))

; #define GetDblTime() (* (unsigned long*) 0x02F0)
(deftrap GetDblTime ("2038" "02f0") ; move.l 0x2f0,d0
  nil
  (long d0))

(deftrap BitClr ("a85f")
  ((bytePtr address)
   (bitnum long))
  nil)

; pascal Boolean BitTst(const void *bytePtr, long bitNum) ONEWORDINLINE(0xA85D);
(deftrap BitTst ("a85d")
  ((bytePtr address)
   (bitnum long))
  char)
