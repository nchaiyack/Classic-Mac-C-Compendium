;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defconst cedit:ok-button 1)
(defconst cedit:cancel-button 2)
(defconst cedit:revert-button 3)
(defconst cedit:defaults-button 4)
(defconst cedit:use-color-button 5)
(defconst cedit:frame-panel 6)
(defconst cedit:back-panel 7)
(defconst cedit:text-panel 8)
(defconst cedit:status-panel 9)
(defconst cedit:status-text-panel 10)
(defconst cedit:cursor-panel 11)
(defconst cedit:cursor-text-panel 12)
(defconst cedit:result-panel 13)
(defconst cedit:red-panel 14)
(defconst cedit:green-panel 15)
(defconst cedit:blue-panel 16)
(defconst cedit:mix-panel 17)
(defconst cedit:dotted-border 18)

(defconst cedit:plt-white 0)
(defconst cedit:plt-black 1)
(defconst cedit:plt-grey 2)
(defconst cedit:plt-back 3)
(defconst cedit:plt-frame 4)
(defconst cedit:plt-text 5)
(defconst cedit:plt-status 6)
(defconst cedit:plt-status-text 7)
(defconst cedit:plt-cursor 8)
(defconst cedit:plt-cursor-text 9)
(defconst cedit:plt-red 10)
(defconst cedit:plt-green 11)
(defconst cedit:plt-blue 12)
(defconst cedit:plt-in-size 10)
(defconst cedit:plt-working-size 13)

(defconst cedit:use-color-pref-index 133)
(defvar cedit:selected cedit:frame-panel)
(defvar cedit:use-color)

(defconst cedit:panel-table
  (list
   (list cedit:frame-panel cedit:plt-frame "Frame color: " cedit:plt-black)
   (list cedit:back-panel cedit:plt-back "Background color: " cedit:plt-white)
   (list cedit:text-panel cedit:plt-text "Text color: " cedit:plt-black)
   (list cedit:status-panel cedit:plt-status "Status line color: " cedit:plt-black)
   (list cedit:status-text-panel cedit:plt-status-text "Status text color: " cedit:plt-white)
   (list cedit:cursor-panel cedit:plt-cursor "Cursor color: " cedit:plt-black)
   (list cedit:cursor-text-panel cedit:plt-cursor-text "Cursor text color: " cedit:plt-white)))

(defconst cedit:animated-entries
  (list cedit:plt-back cedit:plt-frame cedit:plt-text
		cedit:plt-status cedit:plt-status-text cedit:plt-cursor
		cedit:plt-cursor-text))

(defun cedit:in-color ()
  (>= (main-device-depth) 4))

(defun do-edit-colors (menu item)
  (cond
   ((not (have-color-QuickDraw))
	(Alert 139 0))
   ((or (>= (main-device-depth) 4) (= (Alert 140 0) 1))
	(do-edit-colors1))))

(defun do-edit-colors1 ()
  (let* ((type (make-string (c:sizeof 'short) 0))
		 (h (make-string (c:sizeof 'Handle) 0))
		 (box (make-rect))
		 (cedit:palette (cedit:get-pref-palette))
		 (last-click-time 0)
		 (cedit:red-box (make-rect))
		 (cedit:green-box (make-rect))
		 (cedit:blue-box (make-rect))
		 (cedit:mix-box (make-rect))
		 (cedit:dialog (GetNewDialog 134 0 -1))
		 (cedit:levels (make-vector 3 nil))
		 cedit:slider-top cedit:slider-bottom cedit:slider-height)
	
	(ResizePalette cedit:palette cedit:plt-working-size)
	(SetEntryColor cedit:palette cedit:plt-red (make-rgb 65535 0 0))
	(SetEntryColor cedit:palette cedit:plt-green (make-rgb 0 65535 0))
	(SetEntryColor cedit:palette cedit:plt-blue (make-rgb 0 0 65535))
	(NSetPalette cedit:dialog cedit:palette pmFgUpdates)
	(GetDItem cedit:dialog cedit:red-panel type h cedit:red-box)
	(GetDItem cedit:dialog cedit:green-panel type h cedit:green-box)
	(GetDItem cedit:dialog cedit:blue-panel type h cedit:blue-box)
	(GetDItem cedit:dialog cedit:mix-panel type h cedit:mix-box)
	(InsetRect cedit:red-box 1 1)
	(InsetRect cedit:green-box 1 1)
	(InsetRect cedit:blue-box 1 1)
	(InsetRect cedit:mix-box 1 1)
	(setq cedit:slider-top (c:slotref 'Rect cedit:red-box 'top))
	(setq cedit:slider-bottom (c:slotref 'Rect cedit:red-box 'bottom))
	(setq cedit:slider-height (- (c:slotref 'Rect cedit:red-box 'bottom)
								 (c:slotref 'Rect cedit:red-box 'top)))
	(mapcar (function (lambda (x) (SetEntryUsage cedit:palette x pmAnimated 0)))
			cedit:animated-entries)
	(ActivatePalette cedit:dialog)
	
    ;; All user-items will be drawn with draw-panel.
	(setq dialog-user-item-callback-proc-list nil)
	(mapcar (function
			 (lambda (x)
			   (GetDItem cedit:dialog x type h box)
			   (SetDItem cedit:dialog x (extract-internal type 0 'short)
						 dialog-user-item-callback box)
			   (setq dialog-user-item-callback-proc-list
					 (cons (cons x (function draw-panel))
						   dialog-user-item-callback-proc-list))))
			(list cedit:frame-panel
				  cedit:back-panel
				  cedit:text-panel
				  cedit:status-panel
				  cedit:status-text-panel
				  cedit:cursor-panel
				  cedit:cursor-text-panel
				  cedit:result-panel
				  cedit:red-panel
				  cedit:green-panel
				  cedit:blue-panel
				  cedit:mix-panel
				  cedit:dotted-border))
	
	;; Check box must be set correctly.
	(let* ((console-palette (GetPalette (console-WindowPtr)))
		   (entries (cedit:palette-entries console-palette)))
	  (setq cedit:use-color (= entries cedit:plt-in-size)))
	(GetDItem cedit:dialog cedit:use-color-button type h box)
	(let ((ctl-handle (extract-internal h 0 'unsigned-long)))
	  (SetCtlValue ctl-handle (if cedit:use-color 1 0)))
	
	(cedit:set-slider-levels)
	(ShowWindow cedit:dialog)
	(InitCursor)
	(let ((looping t)
		  item-str)
	  (setq item-str (NewPtr 2))
	  (if (zerop (MemError))
		  (progn
			(while looping
			  (ModalDialog (function cedit:filter) item-str)
			  (let* ((item (extract-internal item-str 0 'short))
					 (panel (assoc item cedit:panel-table)))
				(cond
				 (panel
				  (cedit:panel-action))
				 ((= item cedit:mix-panel)
				  (if cedit:use-color
					  (let ((now (TickCount))
							(panel (assoc cedit:selected cedit:panel-table)))
						(if (<= (- now last-click-time) (GetDblTime))
							(cedit:edit-one-color (nth 2 panel) (nth 1 panel))
						  (setq last-click-time now)))))
				 ((= item cedit:red-panel)
				  (if cedit:use-color
					  (cedit:do-slider-action 0 0 cedit:red-box cedit:plt-red)))
				 ((= item cedit:green-panel)
				  (if cedit:use-color
					  (cedit:do-slider-action 2 1 cedit:green-box cedit:plt-green)))
				 ((= item cedit:blue-panel)
				  (if cedit:use-color
					  (cedit:do-slider-action 4 2 cedit:blue-box cedit:plt-blue)))
				 ((= item cedit:ok-button)
				  (cedit:ok-action)
				  (setq looping nil))
				 ((= item cedit:cancel-button)
				  (setq looping nil))
				 ((= item cedit:revert-button)
				  (cedit:revert-defaults-action nil))
				 ((= item cedit:defaults-button)
				  (cedit:revert-defaults-action t))
				 ((= item cedit:use-color-button)
				  (cedit:use-color-action)))))
			(DisposPtr item-str))))
	
	(DisposeDialog cedit:dialog)))

(defun cedit:panel-action ()
  (if cedit:use-color
	  (let ((now (TickCount)))
		(if (<= (- now last-click-time) (GetDblTime))
			(cedit:edit-one-color (nth 2 panel) (nth 1 panel))
		  (setq last-click-time now)
		  (if (not (= cedit:selected item))
			  (let ((old-panel (assoc cedit:selected cedit:panel-table)))
				(SetPort cedit:dialog)
				(GetDItem cedit:dialog cedit:selected type h box)
				(cedit:draw-colored-box old-panel box nil)
				(setq cedit:selected item)
				(GetDItem cedit:dialog cedit:selected type h box)
				(cedit:draw-colored-box panel box t)
				(cedit:set-slider-levels)
				(draw-slider cedit:red-box cedit:plt-red (aref cedit:levels 0))
				(draw-slider cedit:green-box cedit:plt-green (aref cedit:levels 1))
				(draw-slider cedit:blue-box cedit:plt-blue (aref cedit:levels 2))
				(PmForeColor (if (cedit:in-color)
								 (nth 1 (assoc cedit:selected cedit:panel-table))
							   cedit:plt-black))
				(PaintRect cedit:mix-box)
				(PmForeColor cedit:plt-black)))))))

(defun cedit:reset-use-color (new-value)
  (GetDItem cedit:dialog cedit:use-color-button type h box)
  (let ((ctl-handle (extract-internal h 0 'unsigned-long)))
	(SetCtlValue ctl-handle new-value))
  (setq cedit:use-color (not (zerop new-value)))
  (SetPort cedit:dialog)
  (GetDItem cedit:dialog cedit:dotted-border type h box)
  (InvalRect box)
  (GetDItem cedit:dialog cedit:result-panel type h box)
  (InvalRect box))

(defun cedit:use-color-action ()
  (GetDItem cedit:dialog cedit:use-color-button type h box)
  (let* ((ctl-handle (extract-internal h 0 'unsigned-long))
		 (old-value (GetCtlValue ctl-handle))
		 (new-value (if (zerop old-value) 1 0)))
	(cedit:reset-use-color new-value)))

(defun cedit:ok-action ()
  ;; Write out b/w preference.
  (let ((h (NewHandle 2)))
	(if (zerop (MemError))
		(progn
		  (HLock h)
		  (encode-internal (deref h) 0 'short (if cedit:use-color 1 0))
		  (HUnlock h)
		  (set-preference "DATA" cedit:use-color-pref-index h))))
  
  ;; Release all animated entries.
  (mapcar (function (lambda (x) (SetEntryUsage cedit:palette x pmTolerant 0)))
		  cedit:animated-entries)
  
  ;; Set new palette, and release old.
  (ResizePalette cedit:palette (if cedit:use-color cedit:plt-in-size 2))
  (let ((old-palette (GetPalette (console-WindowPtr))))
	(SetPalette (console-WindowPtr) cedit:palette 1)
	(DisposHandle old-palette))
  (ActivatePalette (console-WindowPtr))

  ;; Need to set the window color table.
  (let ((back-color (make-string (c:sizeof 'RGBColor) 0))
		(frame-color (make-string (c:sizeof 'RGBColor) 0))
		(ctab-array (c:getf (c:deref (c:cast 'CTabHandle (console-WCTabHandle)))
							'ctTable))) 
	(GetEntryColor cedit:palette cedit:plt-back back-color)
	(GetEntryColor cedit:palette cedit:plt-frame frame-color)
	(c:set (c:getf (c:aref ctab-array 0) 'rgb) back-color)
	(c:set (c:getf (c:aref ctab-array 1) 'rgb) frame-color)
	(SetWinColor (console-WindowPtr) (console-WCTabHandle)))
  
  ;; Save color palette to prefs file.
  (if cedit:use-color
	  (let ((palette-copy (make-string 4 0)))
		(encode-internal palette-copy 0 'unsigned-long cedit:palette)
		(if (zerop (HandToHand palette-copy))
			(let ((palette-copy-contents (extract-internal palette-copy 0 'unsigned-long)))
			  (set-preference "pltt" 132 palette-copy-contents)))))
  
  ;; Use of direct devices requires an explict update event.
  (SetPort (console-WindowPtr))
  (let ((r (c:value (c:deref (c:getf (c:cast 'GrafPtr (console-WindowPtr))
									 'portRect)))))
	(InvalRect r)))

(defun cedit:revert-defaults-action (default)
  (let ((orig-use-color (if default
							1
						  (let ((pref (get-preference "DATA" 133)))
							(if (< pref 0)
								1
							  (prog2
							   (HLock pref)
							   (extract-internal (deref pref) 0 'short)
							   (HUnlock pref))))))
		(orig-palette (if default
						  (GetNewPalette 136)
						(cedit:get-pref-palette))))
	(cedit:reset-use-color orig-use-color)
	(mapcar (function (lambda (x)
						(let ((c (make-string (c:sizeof 'RGBColor) 0)))
						  (GetEntryColor orig-palette x c)
						  (SetEntryColor cedit:palette x c)
						  (AnimateEntry cedit:dialog x c))))
			cedit:animated-entries)
	(DisposePalette orig-palette))
  (cedit:set-slider-levels)
  (draw-slider cedit:red-box cedit:plt-red (aref cedit:levels 0))
  (draw-slider cedit:green-box cedit:plt-green (aref cedit:levels 1))
  (draw-slider cedit:blue-box cedit:plt-blue (aref cedit:levels 2)))

(defun cedit:do-slider-action (slot-offset level-index box slider-index)
  (let* ((pt (make-string 4 0))
		 (last-pt-v 0)
		 (c (make-string (c:sizeof 'RGBColor) 0))
		 (panel (assoc cedit:selected cedit:panel-table))
		 (target-index (nth 1 panel))
		 (target-item-index (nth 0 panel))
		 (h (make-string 4 0))
		 (type (make-string 2 0))
		 (selected-box (make-rect))
		 (animated-or-direct (cedit:animated-or-direct
							  (cedit:portRect-in-global-coordinates cedit:dialog)))
		 (animated-device (car animated-or-direct))
		 (direct-device (cdr animated-or-direct)))
	(GetDItem cedit:dialog target-item-index type h selected-box)
	(GetEntryColor cedit:palette target-index c)
	(SetPort cedit:dialog)
	(while (not (zerop (WaitMouseUp)))
	  (GetMouse pt)
	  (let ((pt-v (extract-internal pt 0 'short)))
		(if (not (= pt-v last-pt-v))
			(let* ((rgb-level (cedit:screen-to-rgb pt-v))
				   (screen-level (cedit:rgb-to-screen rgb-level)))
			  (encode-internal c slot-offset 'short rgb-level)
			  (aset cedit:levels level-index screen-level)
			  (if animated-device
				  (AnimateEntry cedit:dialog target-index c))
			  (if direct-device
				  (progn
					(SetEntryColor cedit:palette target-index c)
					(PmForeColor target-index)
					(PaintRect cedit:mix-box)
					(PmForeColor cedit:plt-black)
					(cedit:draw-colored-box panel selected-box t)))
		      (draw-slider box slider-index screen-level)
			  (setq last-pt-v pt-v)))))
	(if direct-device (cedit:draw-result))))

(defun cedit:rgb-to-screen (x)
  (- cedit:slider-bottom (/ (* (lsh x -8) cedit:slider-height) 255)))

(defun cedit:screen-to-rgb (x)
  (cond
   ((> x cedit:slider-bottom)
	0)
   ((< x cedit:slider-top)
	65535)
   (t
	(lsh (/ (* (- cedit:slider-bottom x) 255) cedit:slider-height) 8))))

(defun cedit:set-slider-levels ()
  (let ((c (make-string (c:sizeof 'RGBColor) 0)))
	(GetEntryColor cedit:palette (nth 1 (assoc cedit:selected cedit:panel-table)) c)
	(aset cedit:levels 0 (cedit:rgb-to-screen (c:slotref 'RGBColor c 'red)))
	(aset cedit:levels 1 (cedit:rgb-to-screen (c:slotref 'RGBColor c 'green)))
	(aset cedit:levels 2 (cedit:rgb-to-screen (c:slotref 'RGBColor c 'blue)))))

(defun cedit:edit-one-color (prompt index)
  (let ((out-color (make-string 6 0))
		(in-color (make-string 6 0)))
	(GetEntryColor cedit:palette index in-color)
	(let ((GetColor-result
		   (GetColor (make-string 4 0) (CtoPstr prompt) in-color out-color)))
	  (if (not (zerop GetColor-result))
		  (progn
			(AnimateEntry cedit:dialog index out-color)
			(SetEntryColor cedit:palette index out-color)
			(cedit:set-slider-levels)
			(SetPort cedit:dialog)
			(InvalRect cedit:red-box)
			(InvalRect cedit:green-box)
			(InvalRect cedit:blue-box)
			(InvalRect cedit:mix-box))))))

(defun cedit:draw-colored-box (panel box selected)
  (let ((color-index (if cedit:use-color
						 (if (cedit:in-color)
							 (nth 1 panel)
						   cedit:plt-black)
					   (nth 3 panel))))
	(InsetRect box -3 -3)
	(if (or (not cedit:use-color) (not selected))
		(PenMode patBic))
	(FrameRect box)
	(PenNormal)
	(InsetRect box 2 2)
	(FrameRect box)
	(InsetRect box 1 1)
	(PmForeColor color-index)
	(PaintRect box)
	(PmForeColor cedit:plt-black)))

(defun draw-slider (box-all color level)
  (let ((box-white (copy-sequence box-all))
		(box-color (copy-sequence box-all)))
	(encode-internal box-white 4 'short level)
	(PmForeColor cedit:plt-white)
	(PaintRect box-white)
	(encode-internal box-color 0 'short level)
	(PmForeColor (if (cedit:in-color) color cedit:plt-black))
	(PaintRect box-color)
	(PmForeColor cedit:plt-black)))

(defun draw-panel (d item)
  (let* ((type (make-string 2 0))
		 (h (make-string 4 0))
		 (box (make-rect))
		 (panel (assoc item cedit:panel-table)))
    (GetDItem d item type h box)
    (SetPort d)
    (cond
	 (panel
	  (cedit:draw-colored-box panel box (= item cedit:selected)))
	 ((= item cedit:dotted-border)
	  (let ((huge-rgn (NewRgn))
			(check-box-rgn (NewRgn))
			(huge-rect (make-rect))
			(check-box-rect (make-rect)))
		(GetDItem d cedit:use-color-button type h check-box-rect)
		(InsetRect check-box-rect -4 0)
		(SetRect huge-rect -32767 -32767 32767 32767)
		(RectRgn huge-rgn huge-rect)
		(RectRgn check-box-rgn check-box-rect)
		(DiffRgn huge-rgn check-box-rgn check-box-rgn)
		(SetClip check-box-rgn)
		(PenPat gray)
		(FrameRect box)
		(PenNormal)
		(ClipRect huge-rect)
		(DisposeRgn huge-rgn)
		(DisposeRgn check-box-rgn)))
	 ((= item cedit:red-panel)
	  (if cedit:use-color
		  (progn
			(FrameRect box)
			(draw-slider cedit:red-box cedit:plt-red (aref cedit:levels 0)))
		(EraseRect box)))
	 ((= item cedit:green-panel)
	  (if cedit:use-color
		  (progn
			(FrameRect box)
			(draw-slider cedit:green-box cedit:plt-green (aref cedit:levels 1)))
		(EraseRect box)))
	 ((= item cedit:blue-panel)
	  (if cedit:use-color
		  (progn
			(FrameRect box)
			(draw-slider cedit:blue-box cedit:plt-blue (aref cedit:levels 2)))
		(EraseRect box)))
	 ((= item cedit:mix-panel)
	  (if cedit:use-color
		  (progn
			(FrameRect box)
			(InsetRect box 1 1)
			(PmForeColor (if (cedit:in-color)
							 (nth 1 (assoc cedit:selected cedit:panel-table))
						   cedit:plt-black))
			(PaintRect box)
			(PmForeColor cedit:plt-black))
		(EraseRect box)))
     ((= item cedit:result-panel)
	  (cedit:draw-result)))))

(defun cedit:draw-result ()
  (let ((box (make-rect))
		(type (make-string 2 0))
		(h (make-string 4 0)))
    (GetDItem cedit:dialog cedit:result-panel type h box)
	(if (and cedit:use-color (not (cedit:in-color)))
		(progn
		  (PenPat ltGray)
		  (PaintRect box)
		  (PenNormal)
		  (FrameRect box))
	  (let ((box-left (c:slotref 'Rect box 'left))
			(box-top (c:slotref 'Rect box 'top))
			(box-right (c:slotref 'Rect box 'right))
			(box-bottom (c:slotref 'Rect box 'bottom)))
		;; Fill inside
		(PmForeColor (if cedit:use-color cedit:plt-back cedit:plt-white))
		(PaintRect box)
		
		;; Draw frame
		(PmForeColor (if cedit:use-color cedit:plt-frame cedit:plt-black))
		(FrameRect box)
		(MoveTo (- box-right 10) (+ box-top 9)) (LineTo (- box-right 10) (1- box-bottom))
		(MoveTo box-left (+ box-top 10)) (LineTo (1- box-right) (+ box-top 10))
		(MoveTo (- box-right 9) (- box-bottom 10)) (LineTo (1- box-right) (- box-bottom 10))
		
		;; Draw grey areas
		(let* ((top-rect (substring box 0))
			   (bottom-rect (substring box 0)))
		  (InsetRect top-rect 1 1)
		  (InsetRect bottom-rect 1 1)
		  (c:slotset 'Rect top-rect 'bottom (+ box-top 10))
		  (c:slotset 'Rect bottom-rect 'left (- box-right 9))
		  (c:slotset 'Rect bottom-rect 'top (- box-bottom 9))
		  (PmForeColor (if cedit:use-color cedit:plt-grey cedit:plt-white))
		  (PaintRect top-rect)
		  (PaintRect bottom-rect))
		
		;; Draw status bar
		(let ((status-rect (make-rect)))
		  (SetRect status-rect (+ box-left 3) (- box-bottom 20)
				   (- box-right 12) (- box-bottom 11))
		  (PmForeColor (if cedit:use-color cedit:plt-status cedit:plt-black))
		  (PaintRect status-rect)
		  (MoveTo (1+ (c:slotref 'Rect status-rect 'left))
				  (1- (c:slotref 'Rect status-rect 'bottom)))
		  (PmForeColor (if cedit:use-color cedit:plt-status-text cedit:plt-white))
		  (TextFont monaco)
		  (TextSize 9)
		  (DrawString (CtoPstr "--**--Emacs: file")))
		
		;; Draw text
		(MoveTo (+ box-left 4) (+ box-top 21))
		(PmForeColor (if cedit:use-color cedit:plt-text cedit:plt-black))
		(DrawString (CtoPstr "Sample text"))
		
		(let ((cursor-rect (make-rect)))
		  (SetRect cursor-rect 0 0 6 11)
		  (OffsetRect cursor-rect (+ box-left 16) (+ box-top 12))
		  (PmForeColor (if cedit:use-color cedit:plt-cursor cedit:plt-black))
		  (PaintRect cursor-rect)
		  (MoveTo (+ box-left 16) (+ box-top 21))
		  (PmForeColor (if cedit:use-color cedit:plt-cursor-text cedit:plt-white))
		  (DrawChar (string-to-char "m")))
		
		(TextFont 0)
		(TextSize 12)
		(PmForeColor cedit:plt-black)))))

(defun cedit:filter (d e i)
  (let ((what (c:slotref 'EventRecord e 'what)))
    (cond
	 ((= what updateEvt)
	  (let ((type (make-string (c:sizeof 'short) 0))
			(h (make-string (c:sizeof 'long) 0))
			(box (make-rect)))
		(GetDItem d cedit:ok-button type h box)
		(InsetRect box -4 -4)
		(SetPort d)
		(PenSize 3 3)
		(FrameRoundRect box 16 16)
		(PenNormal)
		0))
     ((= what keyDown)
      (let ((c (logand (c:slotref 'EventRecord e 'message) charCodeMask))
			(modifiers (c:slotref 'EventRecord e 'modifiers)))
		(cond
		 ((or (= c (string-to-char "\r")) (= c 3))
		  (encode-internal i 0 'short cedit:ok-button)
		  (blink d cedit:ok-button)
		  1)
		 ((and (= c (string-to-char ".")) (not (zerop (logand modifiers cmdKey))))
		  (encode-internal i 0 'short cedit:cancel-button)
		  (blink d cedit:cancel-button)
		  1)
		 (t
		  0))))
     (t
      0))))

(defun cedit:palette-entries (ph)
  (prog2
   (HLock ph)
   (c:value (c:deref (c:getf (c:deref (c:cast 'PaletteHandle ph)) 'pmEntries)))
   (HUnlock ph)))

(defun cedit:get-pref-palette ()
  (let ((pref (get-preference "pltt" 132)))
	(if (< pref 0)
		(GetNewPalette 136)
	  (if (= (cedit:palette-entries pref)
			 (1- cedit:plt-in-size))
		  (let ((c (make-string (c:sizeof 'RGBColor) 0)))
			(ResizePalette pref cedit:plt-in-size)
			(GetEntryColor pref cedit:plt-text c)
			(SetEntryColor pref cedit:plt-cursor-text c)))
	  pref)))

(defun cedit:portRect-in-global-coordinates (w)
  (let ((r (c:value (c:deref (c:getf (c:cast 'GrafPtr w) 'portRect))))
		(pt (make-string (c:sizeof 'Point) 0)))
	(SetPort w)
	(LocalToGlobal pt)
	(let ((pt-h (extract-internal pt 2 'short))
		  (pt-v (extract-internal pt 0 'short)))
	  (OffsetRect r pt-h pt-v)
	  r)))

(defun cedit:animated-or-direct (focus-rect)
  (let ((device (GetDeviceList))
		(common-rect (make-rect))
		(animated nil)
		(direct nil))
	(while (not (zerop device))
	  (let ((device-rect
			 (c:value (c:deref (c:getf (c:deref (c:cast 'GDHandle device))
									   'gdRect)))))
		(if (not (zerop (SectRect focus-rect device-rect common-rect)))
			(let ((device-type
				   (c:value (c:deref (c:getf (c:deref (c:cast 'GDHandle device))
									   'gdType)))))
			  (cond
			   ((= device-type clutType)
				(setq animated t))
			   ((= device-type directType)
				(setq direct t)))))
	  (setq device (GetNextDevice device))))
	(cons animated direct)))
