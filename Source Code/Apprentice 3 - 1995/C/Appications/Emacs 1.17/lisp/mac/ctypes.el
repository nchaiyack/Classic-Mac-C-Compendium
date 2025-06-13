;;;
;;; This file is part of a Macintosh port of GNU Emacs.
;;; Copyright (C) 1993, 1994 Marc Parmet.  All rights reserved.
;;;
;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;

(setq c:type-list '((char base 1 char) (short base 2 short) (long base 4 long)
					(unsigned-char base 1 unsigned-char)
					(unsigned-short base 2 unsigned-short)
					(unsigned-long base 4 unsigned-long)))

(defun c:sizeof (type)
  (cond
   ((symbolp type)
    (let ((a (c:lookup-type type)))
      (if (null a) (error "Can't compute size of type: %s" type))
      (c:sizeof a)))
   ((eq (nth 0 type) 'base)
    (nth 1 type))
   ((eq (nth 0 type) 'ptr)
    4)
   ((eq (nth 0 type) 'array)
    (* (c:sizeof (nth 1 type)) (nth 2 type)))
   ((eq (nth 0 type) 'struct)
    (nth 1 type))
   (t
    (error "Illegal type %s" type))))

(defmacro c:defstruct (name fields)
  (list 'c:parse-defstruct (list 'quote name) (list 'quote fields)))

(defmacro c:typedef (new-name existing-name)
  (list 'c:parse-typedef (list 'quote new-name) (list 'quote existing-name)))

(defun c:parse-typedef (new-name existing-name)
  (setq c:type-list (cons (list new-name 'typedef existing-name) c:type-list)))

(defun c:lookup-type (type)
  (let ((a (assoc type c:type-list)))
    (cond
     ((null a)
	  nil)
     ((eq (nth 1 a) 'typedef)
      (if (symbolp (nth 2 a))
		  (c:lookup-type (nth 2 a))
		(nth 2 a)))
     (t
      (cdr a)))))

(defun c:extract-type (type)
  (if (symbolp type)
      (c:lookup-type type)
    type))

(defun c:parse-defstruct (name fields)
  (let* ((fields (c:parse-defstruct-internal fields 0))
		 (field-list (car fields))
		 (total-size (cdr fields)))
    (setq c:type-list (cons (list name 'struct total-size field-list) c:type-list))
    nil))

(defun c:parse-defstruct-internal (fields offset)
  (if (null fields)
      (cons nil offset)
    (let* ((field (car fields))
		   (type (nth 0 field))
		   (name (nth 1 field))
		   (cdr-result (c:parse-defstruct-internal (cdr fields) (+ offset (c:sizeof type)))))
      (cons (cons (list name offset type) (car cdr-result))
			(cdr cdr-result)))))

(defun c:getf (arg1 arg2 &optional arg3)
  (let* ((type (if arg3 arg1 (nth 0 arg1)))
		 (base (if arg3 arg2 (nth 1 arg1)))
		 (offset (if arg3 0 (nth 2 arg1)))
		 (field (if arg3 arg3 arg2))
		 (start-type (c:extract-type type)))
    (if (or (not (consp start-type))
			(not (eq 'ptr (car start-type))))
		(error "Illegal type in c:getf: %s" type))
    (let ((struct-type (c:extract-type (cdr start-type))))
      (if (null struct-type)
		  (error "Structure type not defined: %s" (cdr type)))
      (if (not (eq 'struct (car struct-type)))
		  (error "Type has no members: %s" struct-type))
      (let* ((fields (nth 2 struct-type))
			 (field-desc (assoc field fields)))
		(if (null field-desc) (error "Unknown field %s of type %s" field type))
		(list (cons 'ptr (nth 2 field-desc)) base (+ offset (nth 1 field-desc)))))))

(defun c:aref (arg1 index)
  (let ((type (nth 0 arg1))
		(base (nth 1 arg1))
		(offset (nth 2 arg1)))
    (list (cons 'ptr (nth 2 type)) base (+ offset (* index (c:sizeof (nth 2 type)))))))

(defun c:deref (arg1)
  (let* ((type (nth 0 arg1))
		 (base (nth 1 arg1))
		 (offset (nth 2 arg1))
		 (input-type (c:extract-type type)))
    (if (not (eq 'ptr (car input-type)))
		(error "Type passed to c:deref not a pointer: %s" type))
    (let* ((target-type (c:extract-type (cdr input-type)))
		   (extract-type (if (eq (car target-type) 'base)
							 (nth 2 target-type)
						   (if (eq (car target-type) 'ptr) 'unsigned-long 'string))))
      (list target-type
			(extract-internal base offset extract-type 
							  (if (eq extract-type 'string)
								  (c:sizeof target-type)
								nil))
			0))))

(defun c:set (arg1 data)
  (let* ((type (nth 0 arg1))
		 (base (nth 1 arg1))
		 (offset (nth 2 arg1))
		 (input-type (c:extract-type type)))
    (if (not (eq 'ptr (car input-type)))
		(error "Type passed to c:set not a pointer: %s" type))
    (let* ((target-type (c:extract-type (cdr input-type)))
		   (encode-type (if (eq (car target-type) 'base)
							(nth 2 target-type)
						  (if (eq (car target-type) 'ptr) 'unsigned-long 'string))))
      (encode-internal base offset encode-type data)
      nil)))

(defun c:slotref (type data slot)
  (nth 1 (c:deref (c:getf (c:cast (cons 'ptr type) data) slot))))

(defun c:slotset (type data slot value)
  (c:set (c:getf (c:cast (cons 'ptr type) data) slot) value))

(defun c:cast (type object)
  (list type object 0))

(defun c:value (x)
  (nth 1 x))

(c:typedef Fixed long)
(c:typedef Ptr (ptr . char))
(c:typedef Handle (ptr ptr . char))

(c:defstruct FSSpec ((short vRefNum)
					 (long parID)
					 ((array unsigned-char 64) name)))
