
; banjo sound driver
; Joe Kennedy 2023

mpnl_note_on:

	; get and store midi number
	ld (ix + channel.midi_note), a

	; preserve hl
	ex de, hl

	; use as offset into lookup table
	add a, <pv_tone_lookup
	ld l, a
	adc a, >pv_tone_lookup
	sub a, l
	ld h, a

	; get fnum into l
	ld l, (hl)

	; restore hl, fnum is in de
	ex de, hl

	; decide where to put fnum depending on portamento mode
	ld a, (ix + channel.slide_type)
	cp a, SLIDE_TYPE_PORTA
	jr nz, mnon_no_portamento

		; portamento on, put fnum in target_freq
		ld (ix + channel.target_freq), e

		; check high byte of freq
		; if it's 0xff that implies no notes have played so far
		; so we write to freq too
		ld a, (ix + channel.freq)
		cp a, 0xff
		jr nz, mnon_done
			
	mnon_no_portamento:

		; portamento off, put fnum in freq
		ld (ix + channel.freq), e

	mnon_done:

	; mark pitch and volume as requiring update
	; set note-on bit
	ld a, (ix + channel.flags)
	or a, CHAN_FLAG_PITCH_VOLUME_CHANGED | CHAN_FLAG_NOTE_ON
	ld (ix + channel.flags), a

	; if legato is on, we don't need to reset the macro
	bit CHAN_FLAG_BIT_LEGATO, a
	jp nz, mpnl_command_done

		; reset macro positions
		xor a
		ld (ix + channel.volume_macro_pos), a
		ld (ix + channel.vibrato_counter), a

		jp mpnl_command_done

mpnl_note_off:

	; clear note-on bit
    res CHAN_FLAG_BIT_NOTE_ON, (ix + channel.flags)

	; don't write to registers if channel muted
	bit CHAN_FLAG_BIT_MUTED, (ix + channel.flags)
	jp nz, mpnl_command_done_one_byte_command

		ld a, (ix + channel.subchannel)
		add a, PV_CH1
		ld c, a

		ld a, 63
		out (c), a

		jp mpnl_command_done_one_byte_command

mpnl_xor_mode_on:

	ld a, 3
	out (PV_SOUND_CONTROL), a

	jp mpnl_command_done_one_byte_command
	
mpnl_xor_mode_off:

	ld a, 2
	out (PV_SOUND_CONTROL), a

	jp mpnl_command_done_one_byte_command

; a: instrument number
; hl: pattern pointer
mpnl_instrument_change:
			
	; check if the channel is muted and skip command if it is
	bit CHAN_FLAG_BIT_MUTED, (ix + channel.flags)
	jp nz, mpnl_skip_2_byte_command

	; don't do anything if the instrument number is the same
	cp a, (ix + channel.instrument_num)
	jr z, mpnl_instrument_change_done
		
		; preserve hl
		ex de, hl

		; update the instrument
		call music_instrument_change
		
		; restore hl
		ex de, hl

	mpnl_instrument_change_done:

	jp mpnl_command_done

; a: new volume
; hl: pattern pointer
mpnl_volume_change:

	; store new channel volume
	ld (ix + channel.volume), a

	; set bit to indicate a channel volume change event
	set CHAN_FLAG_BIT_PITCH_VOLUME_CHANGED, (ix + channel.flags)

	jp mpnl_command_done

; a: slide amount
; hl: pattern pointer
mpnl_pitch_slide_up:
	
	; set slide_type = 1
	ld c, SLIDE_TYPE_UP

	jr mpnl_slide_common

; a: slide amount
; hl: pattern pointer
mpnl_pitch_slide_down:
	
	; set slide_type = 2
	ld c, SLIDE_TYPE_DOWN

	jr mpnl_slide_common

; a: portamento amount
; hl: pattern pointer
mpnl_portamento:

	; set slide_type = 3
	ld c, SLIDE_TYPE_PORTA

mpnl_slide_common:

	ld (ix + channel.slide_type), c
	
	; load portamento amount
	ld (ix + channel.slide_amount), a

	; set pitch slide flag
	set CHAN_FLAG_BIT_SLIDE, (ix + channel.flags)

	jp mpnl_command_done


mpnl_slide_off:

	; clear slide
	;ld (ix + channel.slide_amount), 0
	ld (ix + channel.slide_type), 0

	; clear slide flag
	res CHAN_FLAG_BIT_SLIDE, (ix + channel.flags)

	; mark pitch as requiring update
	set CHAN_FLAG_BIT_PITCH_VOLUME_CHANGED, (ix + channel.flags)

	jp mpnl_command_done_one_byte_command


; a: arpeggio values
; hl: pattern pointer
mpnl_arpeggio:

	; load arpeggio offsets
	ld (ix + channel.arpeggio), a

	; set arpeggio position to 3
	ld (ix + channel.arpeggio_pos), 3

	; set arpeggio flag
	set CHAN_FLAG_BIT_ARPEGGIO, (ix + channel.flags)

	jp mpnl_command_done


mpnl_arpeggio_off:

	; clear arpeggio
	;ld (ix + channel.arpeggio_pos), 0
	;ld (ix + channel.arpeggio), 0

	; clear arpeggio flag
	res CHAN_FLAG_BIT_ARPEGGIO, (ix + channel.flags)

	; mark pitch as requiring update
	set CHAN_FLAG_BIT_PITCH_VOLUME_CHANGED, (ix + channel.flags)

	jp mpnl_command_done_one_byte_command


; a: vibrato amount
; hl: pattern pointer
mpnl_vibrato:

	; initialise vibrato counter
	ld (ix + channel.vibrato_counter), 0

	; get vibrato counter add amount
	ld (ix + channel.vibrato_counter_add), a
	
	; load full vibrato amplitude into vibrato_depth
	inc hl
	ld a, (hl)
	ld (ix + channel.vibrato_depth), a

	; set vibrato flag
	set CHAN_FLAG_BIT_VIBRATO, (ix + channel.flags)

	jp mpnl_command_done


mpnl_vibrato_off:

	; clear vibrato flag
	res CHAN_FLAG_BIT_VIBRATO, (ix + channel.flags)

	; mark pitch as requiring update
	set CHAN_FLAG_BIT_PITCH_VOLUME_CHANGED, (ix + channel.flags)

	jp mpnl_command_done_one_byte_command


mpnl_legato_on:

	; switch legato flag on
	set CHAN_FLAG_BIT_LEGATO, (ix + channel.flags)

	jp mpnl_command_done_one_byte_command


mpnl_legato_off:

	; switch legato flag off
	res CHAN_FLAG_BIT_LEGATO, (ix + channel.flags)

	jp mpnl_command_done_one_byte_command


mpnl_order_jump:

	; get order to jump to and store it
	ld (iy + music_state.order_jump), a

	jp mpnl_command_done


mpnl_order_next:

	; get order to jump to by incrementing to the current order
	ld a, (iy + music_state.order)
    inc a

    ; check it's a valid order
    cp a, (iy + music_state.orders_length)
    jr c, mpnlon_order_ok

        ld a, 0

    mpnlon_order_ok:
	ld (iy + music_state.order_jump), a

	jp mpnl_command_done_one_byte_command


; a: new speed
; hl: pattern pointer
mpnl_set_speed_1:

	; get new speed
	ld (iy + music_state.speed_1), a

	jp mpnl_command_done


; a: new speed
; hl: pattern pointer
mpnl_set_speed_2:

	; get new speed
	ld (iy + music_state.speed_2), a

	jp mpnl_command_done


; a: delay amount
; hl: pattern pointer
mpnl_note_delay:

	; get new note delay
	ld (ix + channel.tic_wait), a

	; store the pointer to the next command and return
	; this will be picked up when tic_wait hits zero
	inc hl
	ld (ix + channel.pattern_ptr), l
	ld (ix + channel.pattern_ptr + 1), h

	ret


; skip commands of various sizes
; used for channel types which don't support a command
mpnl_skip_1_byte_command:

	jp mpnl_command_done_one_byte_command


mpnl_skip_2_byte_command:

	jp mpnl_command_done


mpnl_skip_3_byte_command:

	inc hl

	jp mpnl_command_done

