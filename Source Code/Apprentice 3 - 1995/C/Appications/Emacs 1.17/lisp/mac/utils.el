;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(defun bring-emacs-to-the-front ()
  (let ((us (make-string (c:sizeof 'ProcessSerialNumber) 0)))
    (c:slotset 'ProcessSerialNumber us 'highLongOfPSN 0)
    (c:slotset 'ProcessSerialNumber us 'lowLongOfPSN kCurrentProcess)
    (SetFrontProcess us)))

(defun hex-string-to-int (string)
  (cond
   ((numberp string)
	string)
   ((zerop (length string))
	0)
   (t
    (let* ((c (string-to-char (substring string -1)))
		   (place (cond
				   ((and (>= c (string-to-char "0")) (<= c (string-to-char "9")))
					(- c (string-to-char "0")))
				   ((and (>= c (string-to-char "a")) (<= c (string-to-char "f")))
					(+ 10 (- c (string-to-char "a"))))
				   ((and (>= c (string-to-char "A")) (<= c (string-to-char "F")))
					(+ 10 (- c (string-to-char "A"))))
				   (t
					0))))
      (+ place (* 16 (hex-string-to-int (substring string 0 -1))))))))

(defun encode-long-integer (i)
  (let ((s (make-string 4 0)))
    (encode-internal s 0 'long i)
    s))

(defun PtoCstr (s)
  (let* ((n (extract-internal s 0 'unsigned-char)))
    (substring s 1 (1+ n))))

(defun CtoPstr (s)
  (let* ((n (length s))
		 (m (if (> n 255) 255 n))
		 (u (concat " " s)))
    (encode-internal u 0 'char m)
    u))

(defun nmapcar (f &rest x)
  (if (null (car x))
	  nil
    (cons (apply f (mapcar 'car x))
		  (apply 'nmapcar f (mapcar 'cdr x)))))

(defun make-rgb (r g b)
  (let ((s (make-string (c:sizeof 'RGBColor) 0)))
    (c:slotset 'RGBColor s 'red r)
    (c:slotset 'RGBColor s 'green g)
    (c:slotset 'RGBColor s 'blue b)
    s))

(defun make-rect ()
  (make-string (c:sizeof 'Rect) 0))
