;;;	usage: 
;;;		pkmn_etat(x, y)
;;;		pkmn_etat(x)
;;;
;;;		y = numero du pokemon (1 - 6)
;;;		x = etat ("psn, bru, som, gel, par")

;;;	le nom du cheat est affiché automatiquement devant le prototype
;;;	le prototype est affiché automatiquement lors de l'affichage de l'aide
.cheat_name		"pkmn_etat"
.prototype		"(*int* pokemon_number, *string* etat)"
.help			"pokemon_number = [1-6]", 10, "etat = \"psn\", \"par\", \"gel\", \"bru\", \"som\", \"ok\""

;;; .parse = arguments obligatoires (void = pas d'arg)
;;; .parse_optionnal = arguments facultatifs
;;; .argc = recuperer le nombre d'arguments
.parse				offset = __int8
.parse_optionnal	p = __string
.argc				argc

;;; .var8, .var16 = creer un pointeur sur
;;;					une zone de 8 ou 16 bits
.var16				status

;;;	le code commence a .start (plus de directives autorisees apres (. ou %)
.start:
	.autopush
	ld	A, (argc)


	;;; calcul offset ;;;
	ld		A, (offset)
	dec		A			; arg1
	ld		B, 44		; arg2
	call	__mul8		; mul -> A = A * B

	;;; add offset to status_addr ;;;
	ld		HL, 0xd12f	; HL = status_base_addr
	add		L			; HL += __int8 offset
	ld		L, A
	jrnc	_save_status
	inc		H
_save_status:
	ld		(status), HL

	ld		A, (argc)
	cmp		1
	jrz		_set_normal


;;;;;;; strncmp(p, "psn", 3) || strncmp(p, "PSN", 3)
ld		HL, p		; HL = __string p
ld		B, 3		; B = "psn".length
ld		DE, _psn	; DE = "psn"
call	__strncmp	; strncmp(HL, DE, B);
cmp		0
jrz		_set_psn

ld		DE, _PSN	; DE = "PSN"
call	__strncmp
cmp		0
jrz		_set_psn

;;;;;;; strncmp(p, "par", 3) || strncmp(p, "PAR", 3)
ld		DE, _par	; DE = "par"
call	__strncmp	; strncmp(HL, DE, B);
cmp		0
jrz		_set_par

ld		DE, _PAR	; DE = "PAR"
call	__strncmp
cmp		0
jrz		_set_par

;;;;;;; strncmp(p, "gel", 3) || strncmp(p, "GEL", 3)
ld		DE, _gel	; DE = "gel"
call	__strncmp	; strncmp(HL, DE, B);
cmp		0
jrz		_set_gel

ld		DE, _GEL	; DE = "GEL"
call	__strncmp
cmp		0
jrz		_set_gel

;;;;;;; strncmp(p, "bru", 3) || strncmp(p, "BRU", 3)
ld		DE, _bru	; DE = "bru"
call	__strncmp	; strncmp(HL, DE, B);
cmp		0
jrz		_set_bru

ld		DE, _BRU	; DE = "BRU"
call	__strncmp
cmp		0
jrz		_set_bru

;;;;;;; strncmp(p, "som", 3) || strncmp(p, "SOM", 3)
ld		DE, _som	; DE = "som"
call	__strncmp	; strncmp(HL, DE, B);
cmp		0
jrz		_set_som

ld		DE, _SOM	; DE = "SOM"
call	__strncmp
cmp		0
jrz		_set_som

_error:
	.autopop
	ret


_set_psn:
	ld	A, 0x8
	jr	_end
_set_par:
	ld	A, 0x40
	jr	_end
_set_gel:
	ld	A, 0x20
	jr	_end
_set_bru:
	ld	A, 0x10
	jr	_end
_set_som:
	ld	A, 0x4
	jr	_end
_set_normal:
	xor	A

_end:
	ld (status), A
	.autopop
	ret

_psn:	.asciz	"psn"
_PSN:	.asciz	"PSN"
_par:	.asciz	"par"
_PAR:	.asciz	"PAR"
_gel:	.asciz	"gel"
_GEL:	.asciz	"GEL"
_bru:	.asciz	"bru"
_BRU:	.asciz	"BRU"
_som:	.asciz	"som"
_SOM:	.asciz	"SOM"
_ERROR:	.asciz	"Unknown argument"
