
; banjo sound driver
; Joe Kennedy 2024

; hl: pointer to music data
banjo_play_sfx:

	; try to ensure we're playing valid data
	ld a, (hl)
	cp a, BANJO_MAGIC_BYTE
	ret nz

		; check if the sfx flag is set on this tune
		inc hl
		inc hl
        inc hl

		bit STATE_FLAG_BIT_SFX, (hl)
		ret z

			; move pointer back
			dec hl
			dec hl
            dec hl

			; mark sfx as playing
			ld a, 1
			ld (sfx_playing), a

			push iy
			push ix

			ld ix, sfx_channel
			ld iy, sfx_state
			call music_play

            ; set proper subchannel
            ld a, (iy + music_state.sfx_subchannel)
            ld (ix + channel.subchannel), a

            ; mute the channel this sfx uses
            call banjo_mute_song_channel

			pop ix
			pop iy

			ret


banjo_sfx_stop:

    ; do nothing if song isn't playing
    ld a, (sfx_playing)
    or a, a
    ret z

        ; mute this channel
        ld a, (sfx_channel + channel.subchannel)
        add a, PV_CH1
        ld c, a

        ld a, 63
        out (c), a

        ; clear sfx_playing flag
        xor a, a
        ld (sfx_playing), a

        ; unmute this channel
        ld a, (sfx_channel + music_state.sfx_subchannel)
        call banjo_unmute_song_channel

        ret


banjo_update_sfx:

	; check if a song is playing
	; return if it's not
	ld a, (sfx_playing)
	or a, a
	ret z

		; if it is, run the updates
		push ix
		push iy

        ld ix, sfx_channel
        ld iy, sfx_state
		call music_update

		pop iy
		pop ix

		ret


