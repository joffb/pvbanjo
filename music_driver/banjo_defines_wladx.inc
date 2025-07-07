.ifndef BANJO_DEFINES
.define BANJO_DEFINES 1
.define NOTE_ON		0x0
.define NOTE_OFF		0x1
.define INSTRUMENT_CHANGE		0x2
.define VOLUME_CHANGE		0x3
.define XOR_MODE_ON		0x4
.define XOR_MODE_OFF		0x5
.define SLIDE_UP		0x6
.define SLIDE_DOWN		0x7
.define SLIDE_PORTA		0x8
.define SLIDE_OFF		0x9
.define ARPEGGIO		0xa
.define ARPEGGIO_OFF		0xb
.define VIBRATO		0xc
.define VIBRATO_OFF		0xd
.define LEGATO_ON		0xe
.define LEGATO_OFF		0xf
.define ORDER_JUMP		0x10
.define SET_SPEED_1		0x11
.define SET_SPEED_2		0x12
.define ORDER_NEXT		0x13
.define NOTE_DELAY		0x14
.define END_LINE		0x80

.define PV_CH1		0xf8
.define PV_CH2		0xf9
.define PV_CH3		0xfa
.define PV_SOUND_CONTROL		0xfb

.define SLIDE_TYPE_NONE		0x0
.define SLIDE_TYPE_UP		0x1
.define SLIDE_TYPE_DOWN		0x2
.define SLIDE_TYPE_PORTA		0x3

.define CHAN_FLAG_MUTED		0x1
.define CHAN_FLAG_NOTE_ON		0x2
.define CHAN_FLAG_LEGATO		0x4
.define CHAN_FLAG_VOLUME_MACRO		0x8
.define CHAN_FLAG_VIBRATO		0x10
.define CHAN_FLAG_ARPEGGIO		0x20
.define CHAN_FLAG_SLIDE		0x40
.define CHAN_FLAG_PITCH_VOLUME_CHANGED		0x80

.define CHAN_FLAG_BIT_MUTED		0x0
.define CHAN_FLAG_BIT_NOTE_ON		0x1
.define CHAN_FLAG_BIT_LEGATO		0x2
.define CHAN_FLAG_BIT_VOLUME_MACRO		0x3
.define CHAN_FLAG_BIT_VIBRATO		0x4
.define CHAN_FLAG_BIT_ARPEGGIO		0x5
.define CHAN_FLAG_BIT_SLIDE		0x6
.define CHAN_FLAG_BIT_PITCH_VOLUME_CHANGED		0x7

.define STATE_FLAG_PROCESS_NEW_LINE		0x1
.define STATE_FLAG_LOOP		0x2
.define STATE_FLAG_MASTER_VOLUME_CHANGE		0x4
.define STATE_FLAG_SFX		0x8

.define STATE_FLAG_BIT_PROCESS_NEW_LINE		0x0
.define STATE_FLAG_BIT_LOOP		0x1
.define STATE_FLAG_BIT_MASTER_VOLUME_CHANGE		0x2
.define STATE_FLAG_BIT_SFX		0x3

.define BANJO_MAGIC_BYTE		0xba

.define BANJO_LOOP_OFF		0x0
.define BANJO_LOOP_ON		0x1

.STRUCT instrument
	volume_macro_len: db
	volume_macro_loop: db
	volume_macro_ptr: dw
.ENDST

.STRUCT channel
	flags: db
	subchannel: db		; channel number within the chip
	freq: db		; current fnum/tone of the voice
	target_freq: db		; target fnum/tone used for portamento
	volume: db
	midi_note: db
	instrument_num: db
	slide_amount: db		; how much to add/subtract per tic
	slide_type: db		; type of slide (up/down/portamento)
	vibrato_depth: db
	vibrato_counter: db
	vibrato_counter_add: db
	arpeggio_pos: db
	arpeggio: db
	pattern_ptr: dw		; pointer to the current pattern
	line_wait: db		; wait for this many lines
	tic_wait: db		; wait for this many tics
	volume_macro_len: db
	volume_macro_vol: db
	volume_macro_pos: db
	volume_macro_loop: db
	volume_macro_ptr: dw
.ENDST

.STRUCT music_state
	magic_byte: db		; should be 0xba for a valid song
	bank: db
	channel_count: db
	flags: db
	sfx_subchannel: db
	speed_1: db
	speed_2: db
	pattern_length: db
	orders_length: db
	instrument_ptrs: dw
	order_ptrs: dw
	tic: db
	line: db
	order: db
	order_jump: db
	noise_mode: db
.ENDST



.macro SFX_DEF(SFX_LABEL, SFX_PRIORITY)
    .db SFX_PRIORITY
    .db :SFX_LABEL
    .dw SFX_LABEL
.endm

.macro SONG_DEF(SONG_LABEL)
    .db 0
    .db :SONG_LABEL
    .dw SONG_LABEL
.endm

.macro SONG_CHANNELS(N)
    song_channels: INSTANCEOF channel N
    song_channel_ptrs: ds N * 2
.endm

.endif
