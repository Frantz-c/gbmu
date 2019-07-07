.cheat_name	"badge"

.argc				argc
.parse				p = __string

__start:
	ld	A, (argc)
	cmp	0xff
	jrz	_error

;;; verification de la validite
;;;	de la chaine
	ld		DE, p
_loop:
	ld		A, (DE++)
	cmp		0
	jrz		_set_badges
	cmp		47			; '0' - 1
	sub		A, 
	jrnc	_error
	cmp		57			; '8'
	jrc		_error
	jr		_loop

;;; ajout des badges dans B
	ld		B, 0		; B = badges
	ld		DE, p
_set_badges:
	ld		A,	(DE++)
	cmp		0
	jrz		_end
	sub		49				; - '1'
	jrc		_set_badges		; if '0'

	ld		C, A			; A = 3 * A
	sla		A
	add		C
	ld		HL,	_set_bit
	add		L
	ld		L, A
	jrnc	_next
	inc		H
_next:
	jp		(HL)

;;; ajout du badge demande
_set_bit
	set		0, B
	jr		_set_badges
	set		1, B
	jr		_set_badges
	set		2, B
	jr		_set_badges
	set		3, B
	jr		_set_badges
	set		4, B
	jr		_set_badges
	set		5, B
	jr		_set_badges
	set		6, B
	jr		_set_badges
	set		7, B
	jr		_set_badges

;;; enregistrement des badges en memoire
_end:
	ld	(0xd35b), B
	ret

_error:
	ld		HL, _ERROR
	call	_puts
	ret

_ERROR: .asciz	"error"
