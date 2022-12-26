(defun get-command-line ()
  (or
   #+CLISP *args*
   #+SBCL *posix-argv*
   #+LISPWORKS system:*line-arguments-list*
   #+CMU extensions:*command-line-words*
   nil))

(defun read-file-lines (file-name)
  (with-open-file (file file-name)
    (let ((ret (list)))
      (loop for line = (read-line file nil)
            while line do (push line ret))
      (nreverse ret))
    ))

(defun file-content ()
  (if (\= (length (get-command-line)) 3)
      (progn
        (format t "Bad number of arguments")
        (quit))
      (read-file-lines (cadr (get-command-line)))
      ))

(defun prepare-list ()
  (do
   ((content (file-content) (cdr content))
    (numbers nil)
    (i 0 (+ i 1))
    (current nil)
    )
   ((not (car content)) (nreverse numbers))
;    (push  (cons i (parse-integer (car content))) numbers ) ; Part 1
; Part 2
    (push  (cons i ( * 811589153 (parse-integer (car content))) ) numbers ) ; Part 2
    )
  )

(defun find-in-list (mylist lf)
  (do ((ret nil)
       (i -1 (1+ i))
       (pointer (cons nil mylist) (cdr pointer)))
      ( (or ret (not pointer)) ret)
    (if ( =  (car  (cadr pointer) ) lf )
        (progn
          (setf ret (cons (1+ i) (cadr pointer)))
          (setf (cdr pointer) (cddr pointer))
          )
        )
    )
  )

(defun insert-on-pos (mylist val pos)
  (if (= 0 pos)
      (progn
        (setf mylist (cons val mylist))
        mylist)
      (do ((i 1 (1+ i))
           (entry (cons val nil))
           (pointer mylist (cdr pointer))
           )
          (nil)
        (if (= i pos)
            (progn
              (setf (cdr entry) (cdr pointer))
              (setf (cdr pointer) entry)
              (return-from insert-on-pos mylist))
            ))
      )
  )



(defun countit (coordinates)
  (let ( (start-index  (position-if  #'(lambda (x) (= 0 (cdr x))) coordinates))
        (arr-length (length coordinates)) )
    (+
     (cdr (elt coordinates (mod (+ start-index 1000) arr-length) ))
     (cdr (elt coordinates (mod (+ start-index 2000) arr-length) ))
     (cdr (elt coordinates (mod (+ start-index 3000) arr-length) ))
       )
    )
)


(defun ddd (coordinates)
  (let ((arr-size (length coordinates)))
    (do ((i 0 (1+ i)))
        ((>= i arr-size) coordinates)
      (let ((entry (find-in-list coordinates i) )
            (new-position 0))
        (if (= (car entry) 0) ; Handle first element deletion
            (setf coordinates (cdr coordinates)))
        (setf new-position  (mod (+ (cdr (cdr entry)) (car entry) )  (- arr-size 1) ))
;        (if (and (= new-position 0) (/= 0 (car entry)))
;            (setf new-position (1- arr-size)))
        (setf coordinates (insert-on-pos coordinates (cdr entry) new-position ))

        )
      )
    )
  )


(defvar *aa* (prepare-list))


(do ((i 0 (1+ i)))
    ((>= i 10) nil)
  (setf *aa* (ddd *aa*))
)

(format t "~a ~%" (countit *aa*))
