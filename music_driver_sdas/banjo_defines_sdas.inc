NOTE_ON			.equ		0x0
NOTE_OFF			.equ		0x1
INSTRUMENT_CHANGE			.equ		0x2
VOLUME_CHANGE			.equ		0x3
XOR_MODE_ON			.equ		0x4
XOR_MODE_OFF			.equ		0x5
SLIDE_UP			.equ		0x6
SLIDE_DOWN			.equ		0x7
SLIDE_PORTA			.equ		0x8
SLIDE_OFF			.equ		0x9
ARPEGGIO			.equ		0xa
ARPEGGIO_OFF			.equ		0xb
VIBRATO			.equ		0xc
VIBRATO_OFF			.equ		0xd
LEGATO_ON			.equ		0xe
LEGATO_OFF			.equ		0xf
ORDER_JUMP			.equ		0x10
SET_SPEED_1			.equ		0x11
SET_SPEED_2			.equ		0x12
ORDER_NEXT			.equ		0x13
NOTE_DELAY			.equ		0x14
END_LINE			.equ		0x80

PV_CH1			.equ		0xf8
PV_CH2			.equ		0xf9
PV_CH3			.equ		0xfa
PV_SOUND_CONTROL			.equ		0xfb

SLIDE_TYPE_NONE			.equ		0x0
SLIDE_TYPE_UP			.equ		0x1
SLIDE_TYPE_DOWN			.equ		0x2
SLIDE_TYPE_PORTA			.equ		0x3

CHAN_FLAG_MUTED			.equ		0x1
CHAN_FLAG_NOTE_ON			.equ		0x2
CHAN_FLAG_LEGATO			.equ		0x4
CHAN_FLAG_VOLUME_MACRO			.equ		0x8
CHAN_FLAG_VIBRATO			.equ		0x10
CHAN_FLAG_ARPEGGIO			.equ		0x20
CHAN_FLAG_SLIDE			.equ		0x40
CHAN_FLAG_PITCH_VOLUME_CHANGED			.equ		0x80

CHAN_FLAG_BIT_MUTED			.equ		0x0
CHAN_FLAG_BIT_NOTE_ON			.equ		0x1
CHAN_FLAG_BIT_LEGATO			.equ		0x2
CHAN_FLAG_BIT_VOLUME_MACRO			.equ		0x3
CHAN_FLAG_BIT_VIBRATO			.equ		0x4
CHAN_FLAG_BIT_ARPEGGIO			.equ		0x5
CHAN_FLAG_BIT_SLIDE			.equ		0x6
CHAN_FLAG_BIT_PITCH_VOLUME_CHANGED			.equ		0x7

STATE_FLAG_PROCESS_NEW_LINE			.equ		0x1
STATE_FLAG_LOOP			.equ		0x2
STATE_FLAG_MASTER_VOLUME_CHANGE			.equ		0x4
STATE_FLAG_SFX			.equ		0x8

STATE_FLAG_BIT_PROCESS_NEW_LINE			.equ		0x0
STATE_FLAG_BIT_LOOP			.equ		0x1
STATE_FLAG_BIT_MASTER_VOLUME_CHANGE			.equ		0x2
STATE_FLAG_BIT_SFX			.equ		0x3

BANJO_MAGIC_BYTE			.equ		0xba

BANJO_LOOP_OFF			.equ		0x0
BANJO_LOOP_ON			.equ		0x1

instrument.volume_macro_len .equ 0
instrument.volume_macro_loop .equ 1
instrument.volume_macro_ptr .equ 2
_sizeof_instrument .equ 4

channel.flags .equ 0
channel.subchannel .equ 1		; channel number within the chip
channel.freq .equ 2		; current fnum/tone of the voice
channel.target_freq .equ 3		; target fnum/tone used for portamento
channel.volume .equ 4
channel.midi_note .equ 5
channel.instrument_num .equ 6
channel.slide_amount .equ 7		; how much to add/subtract per tic
channel.slide_type .equ 8		; type of slide (up/down/portamento)
channel.vibrato_depth .equ 9
channel.vibrato_counter .equ 10
channel.vibrato_counter_add .equ 11
channel.arpeggio_pos .equ 12
channel.arpeggio .equ 13
channel.pattern_ptr .equ 14		; pointer to the current pattern
channel.line_wait .equ 16		; wait for this many lines
channel.tic_wait .equ 17		; wait for this many tics
channel.volume_macro_len .equ 18
channel.volume_macro_vol .equ 19
channel.volume_macro_pos .equ 20
channel.volume_macro_loop .equ 21
channel.volume_macro_ptr .equ 22
_sizeof_channel .equ 24

music_state.magic_byte .equ 0		; should be 0xba for a valid song
music_state.bank .equ 1
music_state.channel_count .equ 2
music_state.flags .equ 3
music_state.sfx_subchannel .equ 4
music_state.speed_1 .equ 5
music_state.speed_2 .equ 6
music_state.pattern_length .equ 7
music_state.orders_length .equ 8
music_state.instrument_ptrs .equ 9
music_state.order_ptrs .equ 11
music_state.tic .equ 13
music_state.line .equ 14
music_state.order .equ 15
music_state.order_jump .equ 16
music_state.noise_mode .equ 17
_sizeof_music_state .equ 18

