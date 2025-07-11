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
;;; Mouse support.  These are straight from x-mouse.el.
;;;

(defun x-mouse-select (arg)
  "Select Emacs window the mouse is on."
  (let ((start-w (selected-window))
	(done nil)
	(w (selected-window))
	(rel-coordinate nil))
    (while (and (not done)
		(null (setq rel-coordinate
			    (coordinates-in-window-p arg w))))
      (setq w (next-window w))
      (if (eq w start-w)
	  (setq done t)))
    (select-window w)
    rel-coordinate))

(defun x-mouse-set-point (arg)
  "Select Emacs window mouse is on, and move point to mouse position."
  (let* ((relative-coordinate (x-mouse-select arg))
	 margin-column
	 (rel-x (car relative-coordinate))
	 (rel-y (car (cdr relative-coordinate))))
    (if relative-coordinate
	(let ((prompt-width (if (eq (selected-window) (minibuffer-window))
				minibuffer-prompt-width 0)))
	  (move-to-window-line rel-y)
	  (if (eobp)
	      ;; If text ends before the desired line,
	      ;; always position at end of that line.
	      nil
	    (setq margin-column
		  (if (or truncate-lines (> (window-hscroll) 0))
		      (current-column)
		    ;; If we are using line continuation,
		    ;; compensate if first character on a continuation line
		    ;; does not start precisely at the margin.
		    (- (current-column)
		       (% (current-column) (1- (window-width))))))
	    (move-to-column (+ rel-x (1- (max 1 (window-hscroll)))
			       (if (= (point) 1)
				   (- prompt-width) 0)
			       margin-column)))))))

(defun x-mouse-set-mark (arg)
  "Select Emacs window mouse is on, and set mark at mouse position.
Display cursor at that position for a second."
  (if (x-mouse-select arg)
      (let ((point-save (point)))
	(unwind-protect
	    (progn (x-mouse-set-point arg)
		   (push-mark nil t)
		   (sit-for 1))
	  (goto-char point-save)))))


;;; This was originally hand-coded in C.  I wonder why.
(defun coordinates-in-window-p (positions window)
  "Return non-nil if POSITIONS (a list, (SCREEN-X SCREEN-Y)) is in WINDOW.\n\
Returned value is list of positions expressed\n\
relative to window upper left corner."
  (let* ((xcoord (nth 0 positions))
	 (ycoord (nth 1 positions))
	 (edges (window-edges window))
	 (left (nth 0 edges))
	 (top (nth 1 edges))
	 (right (nth 2 edges))
	 (bottom (nth 3 edges)))
    (if (or (< xcoord left) (>= xcoord (1- right))
	    (< ycoord top) (>= ycoord (1- bottom)))
	nil
      (list (- xcoord left) (- ycoord top)))))
