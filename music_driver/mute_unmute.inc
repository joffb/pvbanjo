
; banjo sound driver
; Joe Kennedy 2023

; a: channel number
music_get_channel_pointer:

    or a, a
    jr nz, bmsc_check_ch1

        ld hl, song_channels
        ret

    bmsc_check_ch1:
    dec a
    jr nz, bmsc_ch2

        ld hl, song_channels + _sizeof_channel
        ret

    bmsc_ch2:

        ld hl, song_channels + (_sizeof_channel * 2)
        ret

; a: channel to mute
banjo_mute_song_channel:

    ; preserve channel number
    ld l, a

    ; check if the song is playing 
    ; do nothing if it isn't
    ld a, (song_playing)
    or a, a
    ret z

    push ix
    push iy

    ld iy, song_state

    ; jumped to from banjo_mute_song_channel_from_sfx in sfx module
    bmsc_sfx_jump:

    ; restore channel number
    ld a, l

    ; stop channel output
    add a, PV_CH1
    ld c, a
    ld a, 63
    out (c), a

    ; get channel pointer in hl
    ld a, l
    call music_get_channel_pointer

    ; set muted flag, clear note-on flag
    set CHAN_FLAG_BIT_MUTED, (hl)
    res CHAN_FLAG_BIT_NOTE_ON, (hl)

    pop iy
    pop ix

    ret

; a: channel to mute
banjo_unmute_song_channel:

    ; get channel pointer in hl
    ld a, l
    call music_get_channel_pointer

    ; clear muted flag
    res CHAN_FLAG_BIT_MUTED, (hl)
    
	ret

banjo_mute_song:

    ; make sure a song is playing
    ld a, (song_playing)
    or a, a
    ret z

        push hl

        ; set channel mute flags
        ld hl, song_channels
        ld de, _sizeof_channel

        ; set channel muted flag
        set CHAN_FLAG_BIT_MUTED, (hl)
        ; move to next channel
        add hl, de

        ; set channel muted flag
        set CHAN_FLAG_BIT_MUTED, (hl)
        ; move to next channel
        add hl, de

        ; set channel muted flag
        set CHAN_FLAG_BIT_MUTED, (hl)
        ; move to next channel
        add hl, de

        ; stop all channel output
        ld a, 63
        out (PV_CH1), a
        out (PV_CH2), a
        out (PV_CH3), a

        banjo_mute_song_continue:

        pop hl

        ret

banjo_unmute_song:

    ; make sure a song is playing
    ld a, (song_playing)
    or a, a
    ret z

        push hl

        ; clear channel mute flags
        ld hl, song_channels
        ld de, _sizeof_channel

        ; set channel muted flag
        res CHAN_FLAG_BIT_MUTED, (hl)
        ; move to next channel
        add hl, de

        ; set channel muted flag
        res CHAN_FLAG_BIT_MUTED, (hl)
        ; move to next channel
        add hl, de

        ; set channel muted flag
        res CHAN_FLAG_BIT_MUTED, (hl)
        ; move to next channel
        add hl, de

        pop hl

        ret