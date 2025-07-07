
; banjo sound driver
; Joe Kennedy 2023

banjo_init:

	; stop all channel output
	ld a, 63
	out (PV_CH1), a
	out (PV_CH2), a
	out (PV_CH3), a

	; enable sound with xor off
	ld a, 2
	out (PV_SOUND_CONTROL), a

    ; no songs should be playing intially
	xor a,a
	ld (song_playing), a
	ld (sfx_playing), a

	; clear first byte which has the magic byte
	ld (song_state), a

    ret
