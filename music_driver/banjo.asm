
; banjo sound driver
; Joe Kennedy 2023

.include "banjo_defines_wladx.inc"

.SECTION "BANJO" free
	.include "init.inc"
	.include "play.inc"
	.include "stop.inc"
	.include "update.inc"
	.include "instrument_change.inc"
	.include "pattern_change.inc"
	.include "process_new_line.inc"
	.include "mute_unmute.inc"
.ENDS

.SECTION "BANJO_FNUMS" free
	.include "fnums_pv.inc"
.ENDS

.SECTION "BANJO_COMMANDS" free
	.include "command_jump_table.inc"
	.include "commands.inc"
.ENDS

.SECTION "BANJO_EFFECTS" free
	.include "arpeggio.inc"
	.include "vibrato.inc"
	.include "volume_macro.inc"
	.include "pitch_slide.inc"
.ENDS

.SECTION "BANJO_SFX" free
	.include "sfx.inc"
.ENDS