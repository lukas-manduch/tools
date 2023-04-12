(defvar *data* nil )

(defun get-file (filename)
  (with-open-file (stream filename)
    (loop for line = (read-line stream nil)
	  while line
	  collect (or (parse-integer line :junk-allowed t) 0 ))))





(defun split-by (my-list splitter)
  (let ((result (list nil )))
    (dolist (x my-list)
      (if (= x splitter)
	(setf result (cons nil  result))
	(progn
	  (setf (car result) (cons x (car result)) )
	  )))
    result
    ))


(print (apply #'max (mapcar #'(lambda  (some-list) (reduce #'+ some-list)) 
			    (split-by (get-file "task1_input.txt") 0))))

