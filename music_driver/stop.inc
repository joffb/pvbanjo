
; banjo sound driver
; Joe Kennedy 2023

; stop the currently playing song and mute all channels
banjo_song_stop:

    ; do nothing if a song isn't playing
    ld a, (song_playing)
    or a, a
    ret z

        ; stop all channel output
        ld a, 63
        out (PV_CH1), a
        out (PV_CH2), a
        out (PV_CH3), a

        ld a, 0
        ld (song_playing), a

        ret