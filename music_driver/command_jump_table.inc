; jumps to code to handle commands
music_command_jump_table_sn:

	.dw mpnl_note_on
	.dw mpnl_note_off
	.dw mpnl_instrument_change
	.dw mpnl_volume_change
	.dw mpnl_xor_mode_on
	.dw mpnl_xor_mode_off
	.dw mpnl_pitch_slide_up
	.dw mpnl_pitch_slide_down
	.dw mpnl_portamento
	.dw mpnl_slide_off
	.dw mpnl_arpeggio
	.dw mpnl_arpeggio_off
	.dw mpnl_vibrato
	.dw mpnl_vibrato_off
	.dw mpnl_legato_on
	.dw mpnl_legato_off
	.dw mpnl_order_jump
	.dw mpnl_set_speed_1
	.dw mpnl_set_speed_2
    .dw mpnl_order_next
    .dw mpnl_note_delay
