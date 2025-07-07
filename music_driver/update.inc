
; banjo sound driver
; Joe Kennedy 2023


; update the song if one is playing
banjo_update_song:

	; check if a song is playing
	; return if it's not
	ld a, (song_playing)
	or a, a
	ret z

		; if it is, run the updates
		push ix
		push iy

		ld ix, song_channels
		ld iy, song_state
		call music_update

		pop iy
		pop ix

		ret

; update function to be called every frame
; ix: channels
; iy: music state
music_update:

	; move to next tic
	dec (iy + music_state.tic)
	
	; check if we need to move to the next line
	jr nz, music_update_tics_done

		; signal that we need to process the next line
		set STATE_FLAG_BIT_PROCESS_NEW_LINE, (iy + music_state.flags)

		; check if we need to jump to a new order
		ld a, (iy + music_state.order_jump)
		cp a, 0xff
		jr z, music_update_no_jump
		
			call music_jump_channels_order_and_pattern
			jr music_update_tics_done

		music_update_no_jump:

		; move to next line
		dec (iy + music_state.line)

		; line count is 0 means a new pattern
		jr z, music_update_pattern_done

			; on to next line in pattern
			ld a, (iy + music_state.line)
			and a, 0x1
			jr nz, music_update_new_line_odd_speed

				ld a, (iy + music_state.speed_1)
				jr music_update_new_line_speed_done

			music_update_new_line_odd_speed:

				ld a, (iy + music_state.speed_2)

			music_update_new_line_speed_done:

			; update tic count with speed
			ld (iy + music_state.tic), a

			; done for now
			jr music_update_tics_done

		music_update_pattern_done:

			ld a, (iy + music_state.order)
			inc a

			; check if we've reached the last order
			cp a, (iy + music_state.orders_length)
			jr z, mu_last_order
			
				; not the last order
				; move on to next pattern
				call music_jump_channels_order_and_pattern
				jr music_update_tics_done
			
			; we've reached the last order
			mu_last_order:

				; check if this music is looping or not
				bit STATE_FLAG_BIT_LOOP, (iy + music_state.flags)
				jr z, mu_no_loop

					; the music does loop - so go back to first patterns
					xor a, a
					call music_jump_channels_order_and_pattern
					jr music_update_tics_done
				
				mu_no_loop:

					; is this an sfx?
					bit STATE_FLAG_BIT_SFX, (iy + music_state.flags)
					jr z, mu_no_loop_song

						call banjo_sfx_stop
						ret

					mu_no_loop_song:

						call banjo_song_stop
						ret
			
	music_update_tics_done:

		; is this an sfx?
		bit STATE_FLAG_BIT_SFX, (iy + music_state.flags)
		jr z, mutd_song

			ld ix, sfx_channel
			call music_update_channel_pv
			
			jr music_update_call_done

		mutd_song:

			ld ix, song_channels
			call music_update_channel_pv

			ld ix, song_channels + _sizeof_channel
			call music_update_channel_pv

			ld ix, song_channels + (_sizeof_channel * 2)
			call music_update_channel_pv

		; then return here
		music_update_call_done:

		; clear process new line flag
		res STATE_FLAG_BIT_PROCESS_NEW_LINE, (iy + music_state.flags)

		; clear master volume change flag
		res STATE_FLAG_BIT_MASTER_VOLUME_CHANGE, (iy + music_state.flags)

		ret



; ix: channel
; iy: music state
music_update_channel_pv:

    ; note delay?
    ld a, (ix + channel.tic_wait)
    or a, a
    jr z, muc_sn_no_note_delay

        ; decrement tic wait and check if it's zero
        dec a
        ld (ix + channel.tic_wait), a
        jr nz, muc_sn_no_note_delay

            ; if it is, process the delayed line data
	        call music_process_new_line

    muc_sn_no_note_delay:

    ; process new line if necessary
	bit STATE_FLAG_BIT_PROCESS_NEW_LINE, (iy + music_state.flags)
    jr z, muc_sn_no_process_new_line

        call music_process_new_line

    muc_sn_no_process_new_line:

    ; get flags in b for later
    ld b, (ix + channel.flags)

    ; check that the note is on and the channel isn't muted
    ld a, CHAN_FLAG_MUTED | CHAN_FLAG_NOTE_ON
    and a, b
    cp a, CHAN_FLAG_NOTE_ON
    ret nz

        ; check if we need to update the pitch/volume of this channel
        ld a, b
		and a, CHAN_FLAG_ARPEGGIO | CHAN_FLAG_SLIDE | CHAN_FLAG_VIBRATO | CHAN_FLAG_VOLUME_MACRO | CHAN_FLAG_PITCH_VOLUME_CHANGED
		call nz, music_update_pitch_registers_sn

        ; clear events
        res CHAN_FLAG_BIT_PITCH_VOLUME_CHANGED, (ix + channel.flags)
        
		ret


; b: current channel number
; iy: music_state
; ix: channel we're updating
music_update_pitch_registers_sn:

	ld b, (ix + channel.flags)

	; get pitch port for channel in c
	ld a, (ix + channel.subchannel)
	add a, PV_CH1
	ld c, a

	; check channel volume
	ld a, (ix + channel.volume)
	or a, a 
	jr z, mvcsn_mute_channel

	; volume macro active?
	bit CHAN_FLAG_BIT_VOLUME_MACRO, b
	jp z, mvcsn_no_vol_macro

        ; process volume macro
        call music_update_volume_macro

		; mute if macro value == 0
		or a, a
		jr z, mvcsn_mute_channel

	mvcsn_no_vol_macro:

	; at this point we know the volume is 1
	; so put the correct value in the pitch register

	; is there a pitch slide happening?
	bit CHAN_FLAG_BIT_SLIDE, b
	jr z, mvcsn_no_slide

		call music_update_pitch_slide_sn
		
		jr mucsn_output_value

	mvcsn_no_slide:

	; run arpeggio if one exists
	bit CHAN_FLAG_BIT_ARPEGGIO, b
	jr z, mucsn_no_arpeggio

		call music_update_arpeggio

		; add midi_note to 0 or arpeggio value
		add a, (ix + channel.midi_note)
		add a, <pv_tone_lookup
		ld l, a
		adc a, >pv_tone_lookup
		sub a, l
		ld h, a

		; get tone number
		ld a, (hl)

		jr mucsn_arpeggio_done

	mucsn_no_arpeggio:

		ld a, (ix + channel.freq)

	mucsn_arpeggio_done:

	; run vibrato if enabled
	bit CHAN_FLAG_BIT_VIBRATO, b
	jr z, mucsn_no_vibrato

		ld e, a
		call music_update_vibrato

	mucsn_no_vibrato:
	mucsn_output_value:

		; output value
		out (c), a

		ret

	mvcsn_mute_channel:

		ld a, 63
		jr mucsn_output_value