
; banjo sound driver
; Joe Kennedy 2024

; for (i = 0; i < 16; i++) { out = []; for (j = 0; j < 16; j++) { out.push(Math.floor(Math.sin((Math.PI/32) * j) * (i + 1) * 2));} console.log ("\t.db " + out.join(",")); }

; quarter of sine table
; 16 rows, one for each vibrato amplitude
; 16 values per row, other quadrants of sine are calculated
music_vibrato_table:
	.db 0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1
    .db 0,0,0,1,1,1,2,2,2,3,3,3,3,3,3,3
    .db 0,0,1,1,2,2,3,3,4,4,4,5,5,5,5,5
    .db 0,0,1,2,3,3,4,5,5,6,6,7,7,7,7,7
    .db 0,0,1,2,3,4,5,6,7,7,8,8,9,9,9,9
    .db 0,1,2,3,4,5,6,7,8,9,9,10,11,11,11,11
    .db 0,1,2,4,5,6,7,8,9,10,11,12,12,13,13,13
    .db 0,1,3,4,6,7,8,10,11,12,13,14,14,15,15,15
    .db 0,1,3,5,6,8,10,11,12,13,14,15,16,17,17,17
    .db 0,1,3,5,7,9,11,12,14,15,16,17,18,19,19,19
    .db 0,2,4,6,8,10,12,13,15,17,18,19,20,21,21,21
    .db 0,2,4,6,9,11,13,15,16,18,19,21,22,22,23,23
    .db 0,2,5,7,9,12,14,16,18,20,21,22,24,24,25,25
    .db 0,2,5,8,10,13,15,17,19,21,23,24,25,26,27,27
    .db 0,2,5,8,11,14,16,19,21,23,24,26,27,28,29,29
    .db 0,3,6,9,12,15,17,20,22,24,26,28,29,30,31,31

; ix: current channel
; iy: music state
music_update_vibrato:

    ; add to counter, wraps around at 64
    ld a, (ix + channel.vibrato_counter)
    add a, (ix + channel.vibrato_counter_add)
    and a, 0x3f

    ; store new value and keep it in d
    ld (ix + channel.vibrato_counter), a
    ld d, a

    ; lower 4 bits value will be used as index 
    ; to look up within a row of music_vibrato_table
    and a, 0xf

    ; reverse index into table when counter is between 16-31 and 48-63
    ; if bit 4 is set (16)
    bit 4, d
    jr z, mvu_no_reverse

        neg
        add a, 0xf

    mvu_no_reverse:

    ; index into the table
    ; vibrato_depth used to select a row within music_vibrato_table
    or a, (ix + channel.vibrato_depth)
    add a, <music_vibrato_table
    ld l, a
    adc a, >music_vibrato_table
    sub a, l
    ld h, a

    ; get the value
    ld a, (hl)

    ; vibrato amount == 0
    or a, a
    jr z, mvu_no_invert

    ; invert value from table when counter is between 32 and 63
    bit 5, d
    jr z, mvu_no_invert

        ; subtract vibrato from tone number
        ld d, a
        ld a, e
        sub a, d

        ; no overflow
        ret nc

            ; overflow
            ld a, 0
            ret

    mvu_no_invert:

        add a, e

        ; check for overflow
        cp a, 63
        ret c
        
            ; overflow
            ld a, 62
            ret