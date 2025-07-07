
; banjo sound driver
; Joe Kennedy 2024


; update the pitch slides or portamento for a channel
; ix: channel
; iy: state
music_update_pitch_slide_sn:

	; offset into jump table by slide type * 2
	ld a, (ix + channel.slide_type)
	dec a
	jr z, music_update_sn_pitch_slide_upward
	dec a
	jr z, music_update_sn_pitch_slide_downward
	dec a
	jr z, music_update_sn_portamento

; slide type == 1, upward slide
music_update_sn_pitch_slide_upward:

	; add slide_amount to freq
	ld a, (ix + channel.freq)
	add a, (ix + channel.slide_amount)

	; keep in range 0-62
	cp a, 63
	jr c, muspsu_no_carry

		ld a, 62

	muspsu_no_carry:

	; update freq
	ld (ix + channel.freq), a

	ret

; slide type == 2, downward slide
music_update_sn_pitch_slide_downward:

	; sub slide_amount from freq
	ld a, (ix + channel.freq)
	sub a, (ix + channel.slide_amount)

	; keep in range 0-62
	jr nc, muspsd_no_carry

		xor a, a

	muspsd_no_carry:

	; update freq
	ld (ix + channel.freq), a

	ret

; handle SN chip portamento
; ix: current channel
; iy: music state
; de: current channel.freq
; outputs updated freq in de
music_update_sn_portamento:

	; get target_freq in hl
	ld h, (ix + channel.target_freq)
	ld l, (ix + channel.freq)

	; subtract target freq from current freq
	ld a, l
	cp a, h
	jr z, musn_porta_end_portamento
	jr nc, musn_porta_current_higher

	musn_porta_current_lower:

		; add slide amount to freq
		add a, (ix + channel.slide_amount)

		; check if we've passed target_freq
		; if z then freq == target and we're done
		; if c then freq > target and we're done
		; if nc then we carry on next loop
		cp a, h
		jr z, musn_porta_end_portamento
		jr nc, musn_porta_end_portamento

			; update freq and return
			ld (ix + channel.freq), a
			ret

	musn_porta_current_higher:

		; sub slide amount from freq
		sub a, (ix + channel.slide_amount)

		; check if we've passed target_freq
		; if z then freq == target and we're done
		; if nc then freq < target and we're done
		; if c then we carry on next loop
		cp a, h
		jr z, musn_porta_end_portamento
		jr c, musn_porta_end_portamento
		
			; update freq and return
			ld (ix + channel.freq), a
			ret

	musn_porta_end_portamento:

		; return target freq
		ld a, h
		ld (ix + channel.freq), a

		; cancel the portamento
		ld (ix + channel.slide_type), 0
		res CHAN_FLAG_BIT_SLIDE, (ix + channel.flags)

		ret