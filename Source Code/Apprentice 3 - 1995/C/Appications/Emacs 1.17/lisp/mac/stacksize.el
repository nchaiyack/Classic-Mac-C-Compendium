;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst stacksize-ok-button 1)
(defconst stacksize-cancel-button 2)
(defconst stacksize-stack-item 3)

(defun do-stacksize (menu item)
  (let ((d (GetNewDialog 130 0 -1))
		item-str
		item-int
		(type (make-string (c:sizeof 'short) 0))
		(h (make-string (c:sizeof 'Handle) 0))
		(s (make-string 256 0))
		(box (make-rect))
		(old-value (let ((pref (get-preference "DATA" 129)))
					 (if (< pref 0)
						 min-stack-size
					   (prog2
						(HLock pref)
						(max min-stack-size (extract-internal (deref pref) 0 'long))
						(HUnlock pref))))))
	(setq item-str (NewPtr 2))
	(if (zerop (MemError))
		(progn
		  (GetDItem d stacksize-stack-item type h box)
		  (SetIText (extract-internal h 0 'unsigned-long)
					(CtoPstr (int-to-string old-value)))
		  (SelIText d stacksize-stack-item 0 32767)
		  (ShowWindow d)
		  (InitCursor)
		  (encode-internal item-str 0 'short 0)
		  (while (progn (setq item-int (extract-internal item-str 0 'short))
						(and (not (= item-int stacksize-ok-button))
							 (not (= item-int stacksize-cancel-button))))
			(ModalDialog (function stacksize-filter) item-str))
		  
		  (GetDItem d stacksize-stack-item type h box)
		  (GetIText (extract-internal h 0 'unsigned-long) s)
		  (if (= item-int stacksize-ok-button)
			  (let ((k (NewHandle (c:sizeof 'long))))
				(if (zerop (MemError))
					(let ((new-pref (max min-stack-size (string-to-int (PtoCstr s)))))
					  (HLock k)
					  (encode-internal (deref k) 0 'unsigned-long new-pref)
					  (HUnlock k)
					  (set-preference "DATA" 129 k)))))))
	
    (DisposeDialog d)))

(defun stacksize-filter (d e i)
  (let ((what (c:slotref 'EventRecord e 'what)))
    (cond
     ((= what updateEvt)
      (let ((type (make-string 2 0))
	    (item (make-string 4 0))
	    (box (make-string 8 0)))
	(GetDItem d stacksize-ok-button type item box)
	(SetPort d)
	(PenSize 3 3)
	(InsetRect box -4 -4)
	(FrameRoundRect box 16 16)
	(PenNormal)
	0))
     ((= what keyDown)
      (let ((c (logand (c:slotref 'EventRecord e 'message) charCodeMask))
	    (modifiers (c:slotref 'EventRecord e 'modifiers)))
	(if (or (= c (string-to-char "\r")) (= c 3))
	    (progn
	      (encode-internal i 0 'short stacksize-ok-button)
	      (blink d stacksize-ok-button)
	      1)
	  (if (and (= c (string-to-char ".")) (not (zerop (logand modifiers cmdKey))))
	    (progn
	      (encode-internal i 0 'short stacksize-cancel-button)
	      (blink d stacksize-cancel-button)
	      1)
	    0))))
     (t
      0))))

(defun blink (d item)
  (let ((type (make-string 2 0))
	(h0 (make-string 4 0))
	(box (make-string 8 0))
	(now (TickCount)))
    (GetDItem d item type h0 box)
    (let ((h (extract-internal h0 0 'unsigned-long)))
      (HiliteControl h inButton)
      (while (< (TickCount) (+ 10 now))
	nil)
      (HiliteControl h 0))))
