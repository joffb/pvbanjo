
; banjo sound driver
; Joe Kennedy 2023

; b: channel number
; de: jump table to handle commands (e.g. music_command_jump_table_sn)
; ix: channels
; iy: state
music_process_new_line:
	
	; don't do anything if line_wait > 0
	ld a, (ix + channel.line_wait)
	or a, a
	jr z, mpnl_no_line_wait

		; update line_wait value and return
		dec a
		ld (ix + channel.line_wait), a
		ret

	mpnl_no_line_wait:

	; get current pattern pointer for this channel
	ld l, (ix + channel.pattern_ptr)
	ld h, (ix + channel.pattern_ptr + 1)
	
	mpnl_loop:
	
		; get command
		ld a, (hl)
		
		; is the end of this line for this channel? (bit 7 set, a >= 0x80)
		; if not, use jump table
		bit 7, a
		jp z, mpnl_use_jump_table

			; end of this line reached
			; get line_wait amount and set it in channel
			and a, 0x7f
			ld (ix + channel.line_wait), a

			; store the pointer to the next command and return
			inc hl
			ld (ix + channel.pattern_ptr), l
			ld (ix + channel.pattern_ptr + 1), h

			ret

		; if not, process the command
		mpnl_use_jump_table:

			; preserve pattern pointer (currently in hl) in de
			; we want to use hl's ability to jump
			ex de, hl

			; address in table is command num * 2
			add a, a

			; use it to offset into the jump table
			; the address of which was in de and is now in hl
			add a, <music_command_jump_table_sn
			ld l, a
			adc a, >music_command_jump_table_sn
			sub a, l
			ld h, a

			; get address from table into hl
			ld a, (hl)
			inc hl
			ld h, (hl)
			ld l, a

			; restore pattern pointer to hl
			; jump address is in de
			ex de, hl

			; move pattern pointer on and read
			; the next byte into a in preparation
			inc hl
			ld a, (hl)

			; jump to jump table address
			push de
			ret

		mpnl_command_done:

			inc hl

		mpnl_command_done_one_byte_command:
			
			jr mpnl_loop
