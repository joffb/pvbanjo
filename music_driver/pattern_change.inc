
; banjo sound driver
; Joe Kennedy 2024

; jump to a given order/pattern
; a: order number to jump to
; ix: channels
; iy: state
music_jump_channels_order_and_pattern:

	; order number in de
	ld e, a
	ld d, 0

	; music_state in hl
	push iy
	pop hl

	; move to order_jump
	ld bc, music_state.order_jump
	add hl, bc

	; reset order_jump to 0xff to indicate we don't need to jump
	ld (hl), 0xff
	dec hl

	; update order index
	ld (hl), a
	dec hl

	; reset line
	ld a, (iy + music_state.pattern_length)
	ld (hl), a
	dec hl

	; reset tic
	ld a, (iy + music_state.speed_1)
	ld (hl), a
	dec hl

	; get pointer to orders list in hl
	ld a, (hl)
	dec hl
	ld l, (hl)
	ld h, a

	; order number in hl, pointer to orders list in de
	ex de, hl

	; we want to calculate
	; order offset = order number * 6
	; hl = order * 2
	add hl, hl

	; bc = order * 2
	ld b, h
	ld c, l

	; hl = (order * 4) + (order * 2)
	add hl, hl
	add hl, bc

	; add address of orders list
	; hl should now point at order_ptrs + (order * 6)
	add hl, de

	; order pointer in de
	ex de, hl

	; channel pointer in hl
	push ix
	pop hl

	; move channel pointer to channel.pattern_ptr
	ld bc, channel.pattern_ptr
	add hl, bc

	; order pointer in hl, channel pointer in de
	ex de, hl

	; using ldi so keep a big value in c so it doesn't affect b
	ld c, 0xff

	; loop through all channels
	ld b, (iy + music_state.channel_count)

	mjcoap_loop:

		; update pattern ptr
		ldi
		ldi

		; set line_wait = 0 for channel
		xor a, a
		ld (de), a

		; move along to next channel
		ld a, _sizeof_channel - (channel.line_wait - channel.pattern_ptr)
		add a, e
		ld e, a
		adc a, d
		sub a, e
		ld d, a

		djnz mjcoap_loop

	ret
	