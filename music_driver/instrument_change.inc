
; banjo sound driver
; Joe Kennedy 2023

; ix: channel
; iy: state
; a: new instrument number
music_instrument_change:

	; preserve bc and de
	push bc
	push de

	; store the new instrument number in the channel
	ld (ix + channel.instrument_num), a
	
	; offset to get instrument pointer
	ld h, 0
	ld l, a
	add hl, hl
	add hl, hl

	; get pointer to instrument 
	ld e, (iy + music_state.instrument_ptrs)
	ld d, (iy + music_state.instrument_ptrs + 1)
	add hl, de

	; get channel struct ptr ix into de
	push ix
	pop de

	; get channel.flags (should be first byte of channel struct)
	; then unset the macro flags and store in b 
	ld a, (de)
	and a, ~(CHAN_FLAG_VOLUME_MACRO)
	ld b, a

	; max value of c so ldi instructions don't affect b
	ld c, 0xff

	; move along to volume macro 
	ld a, channel.volume_macro_len
	add a, e
	ld e, a
	adc a, d
	sub a, e
	ld d, a

	; a = 0
	xor a, a

	; volume macro
	; copy over macro info

	; volume macro length == 0?
	cp a, (hl)
	jr z, mic_no_volume_macro

		; set volume macro flag
		set CHAN_FLAG_BIT_VOLUME_MACRO, b
	
	mic_no_volume_macro:

		; len
		ldi

		; vol
		inc de

		; pos = 0
		ld (de), a
		inc de

		; loop
		ldi

		; ptr
		ldi
		ldi

	; update flags
	ld (ix + channel.flags), b

	; restore de and bc
	pop de
	pop bc
	
	ret
