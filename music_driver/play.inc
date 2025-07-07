
; banjo sound driver
; Joe Kennedy 2023

; hl: pointer to music data
banjo_play_song:

	; try to ensure we're playing valid data
	ld a, (hl)
	cp a, BANJO_MAGIC_BYTE
	ret nz

		; get channnel count
		; abort if this song uses too many channels
		inc hl
		inc hl
		ld a, 3
		cp a, (hl)
		ret c

			; move pointer back
			dec hl
			dec hl

			; mark song as playing
			ld a, 1
			ld (song_playing), a

			push iy
			push ix

			ld ix, song_channels
			ld iy, song_state
			call music_play

			pop ix
			pop iy

			; stop all channel output
			ld a, 63
			out (PV_CH1), a
			out (PV_CH2), a
			out (PV_CH3), a

			ret


; ix: channels
; iy: state
; hl: pointer to music data
music_play:

	; put music state header address into de
	push iy
	pop de

	; copy over song header
	ld bc, _sizeof_music_state
	ldir

	; preserve channel data pointer
	push ix
	pop hl

	; init channel data
	ld b, (iy + music_state.channel_count)
	ld c, 0

	; d should remain 0
	ld d, 0
	
	; a should remain 0xff
	ld a, 0xff
		
	music_play_channel_data:

		; flags = 0
		ld (hl), d
		inc hl

		; subchannel
		ld (hl), c
		inc c
		inc hl

		; freq = 0xff
		ld (hl), a
		inc hl
		inc hl

		; default volume to full
		ld (hl), 1
		inc hl
		inc hl

		; instrument num = 0xff
		ld (hl), a

		; move on to line wait
		ld e, channel.line_wait - channel.instrument_num
		add hl, de

		; default line and tic wait to 0
		ld (hl), d
		inc hl
		ld (hl), d

		; move to next channel
		ld e, _sizeof_channel - channel.tic_wait
		add hl, de

		djnz music_play_channel_data

	; set up pattern
	xor a, a
	call music_jump_channels_order_and_pattern

	; set process new line flag
	set STATE_FLAG_BIT_PROCESS_NEW_LINE, (iy + music_state.flags)

	ret

banjo_song_rewind:

	; check the song state to see if the first byte is the magic byte
	ld a, (song_state + music_state.magic_byte)
	cp a, BANJO_MAGIC_BYTE
	ret nz

		push ix
		push iy

		ld ix, song_channels
		ld iy, song_state
		
		xor a, a
		call music_jump_channels_order_and_pattern

		ld b, (iy + music_state.channel_count)
		ld de, _sizeof_channel

		bsrw_channel_loop:

			ld (ix + channel.flags), a
			ld (ix + channel.line_wait), a

			add ix, de
			djnz bsrw_channel_loop

		pop iy
		pop ix

		ret

; if there's a valid banjo song loaded, resume playback of it
banjo_song_resume:

	; check the song state to see if the first byte is the magic byte
	ld a, (song_state + music_state.magic_byte)
	cp a, BANJO_MAGIC_BYTE
	ret nz

		; resume playback
		ld a, 1
		ld (song_playing), a
	
		ret

; a: loop mode
; 	   0 for loop off
; 	>= 1 for loop on
banjo_set_song_loop_mode:

	ld hl, song_state + music_state.flags

	; check loop mode
	or a, a
	jr nz, bsslm_loop_on

		; looping off
		res STATE_FLAG_BIT_LOOP, (hl)
		ret

	bsslm_loop_on:

		; looping on
		set STATE_FLAG_BIT_LOOP, (hl)
		ret