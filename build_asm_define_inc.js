//
// Builds defines_sdas.inc and defines_wladx.inc for the different asm dialects
// 
// Joe Kennedy 2024
//

const fs = require('fs');

var defines = [
    [
        { name: "NOTE_ON", value: 0x00 },
        { name: "NOTE_OFF", value: 0x01 },
        { name: "INSTRUMENT_CHANGE", value: 0x02 },
        { name: "VOLUME_CHANGE", value: 0x03 },
        { name: "XOR_MODE_ON", value: 0x04 },
        { name: "XOR_MODE_OFF", value: 0x05 },
        { name: "SLIDE_UP", value: 0x06 },
        { name: "SLIDE_DOWN", value: 0x07 },
        { name: "SLIDE_PORTA", value: 0x08 },
        { name: "SLIDE_OFF", value: 0x09 },
        { name: "ARPEGGIO", value: 0x0a },
        { name: "ARPEGGIO_OFF", value: 0x0b },
        { name: "VIBRATO", value: 0x0c },
        { name: "VIBRATO_OFF", value: 0x0d },
        { name: "LEGATO_ON", value: 0x0e },
        { name: "LEGATO_OFF", value: 0x0f },
        { name: "ORDER_JUMP", value: 0x10 },
        { name: "SET_SPEED_1", value: 0x11 },
        { name: "SET_SPEED_2", value: 0x12 },
        { name: "ORDER_NEXT", value: 0x13 },
        { name: "NOTE_DELAY", value: 0x14 },
        { name: "END_LINE", value: 0x80 },
    ],
    [
        { name: "PV_CH1", value: 0xf8 },
        { name: "PV_CH2", value: 0xf9 },
        { name: "PV_CH3", value: 0xfa },
        { name: "PV_SOUND_CONTROL", value: 0xfb },
    ],
    [
        { name: "SLIDE_TYPE_NONE", value: 0x00 },
        { name: "SLIDE_TYPE_UP", value: 0x01 },
        { name: "SLIDE_TYPE_DOWN", value: 0x02 },
        { name: "SLIDE_TYPE_PORTA", value: 0x03 },
    ],
    [
        { name: "CHAN_FLAG_MUTED", value: 0x01 },
        { name: "CHAN_FLAG_NOTE_ON", value: 0x02 },
        { name: "CHAN_FLAG_LEGATO", value: 0x04 },
        { name: "CHAN_FLAG_VOLUME_MACRO", value: 0x8 },
        { name: "CHAN_FLAG_VIBRATO", value: 0x10 },
        { name: "CHAN_FLAG_ARPEGGIO", value: 0x20 },
        { name: "CHAN_FLAG_SLIDE", value: 0x40 },
        { name: "CHAN_FLAG_PITCH_VOLUME_CHANGED", value: 0x80 },
    ],
    [
        { name: "CHAN_FLAG_BIT_MUTED", value: 0 },
        { name: "CHAN_FLAG_BIT_NOTE_ON", value: 1 },
        { name: "CHAN_FLAG_BIT_LEGATO", value: 2 },
        { name: "CHAN_FLAG_BIT_VOLUME_MACRO", value: 3 },
        { name: "CHAN_FLAG_BIT_VIBRATO", value: 4 },
        { name: "CHAN_FLAG_BIT_ARPEGGIO", value: 5 },
        { name: "CHAN_FLAG_BIT_SLIDE", value: 6 },
        { name: "CHAN_FLAG_BIT_PITCH_VOLUME_CHANGED", value: 7 },
    ],
    [
        { name: "STATE_FLAG_PROCESS_NEW_LINE", value: 0x01 },
        { name: "STATE_FLAG_LOOP", value: 0x02 },
        { name: "STATE_FLAG_MASTER_VOLUME_CHANGE", value: 0x04 },
        { name: "STATE_FLAG_SFX", value: 0x08 },
    ],
    [
        { name: "STATE_FLAG_BIT_PROCESS_NEW_LINE", value: 0 },
        { name: "STATE_FLAG_BIT_LOOP", value: 1 },
        { name: "STATE_FLAG_BIT_MASTER_VOLUME_CHANGE", value: 2 },
        { name: "STATE_FLAG_BIT_SFX", value: 3 },
    ],
    [
        { name: "BANJO_MAGIC_BYTE", value: 0xba },
    ],
    [
        { name: "BANJO_LOOP_OFF", value: 0},
        { name: "BANJO_LOOP_ON", value: 1}
    ],
];

var structs = [
    {
        name: "instrument",
        members: [
            { name: "volume_macro_len", size: "db"},
            { name: "volume_macro_loop", size: "db"},
            { name: "volume_macro_ptr", size: "dw"},

            //{ name: "ex_macro_type", size: "db"},
            //{ name: "ex_macro_len", size: "db"},
            //{ name: "ex_macro_loop", size: "db"},
            //{ name: "ex_macro_ptr", size: "dw"},
            
            //{ name: "fm_preset", size: "db" },
            //{ name: "fm_patch", size: "dw" },
        ]
    },
    {
        name: "channel",
        members: [
            { name: "flags", size: "db" },
            { name: "subchannel", size: "db", comment: "channel number within the chip" },
            //{ name: "port", size: "db", comment: "output port, used for certain chips" },
            //{ name: "events", size: "db", comment: "" },

            { name: "freq", size: "db", comment: "current fnum/tone of the voice" },
            { name: "target_freq", size: "db", comment: "target fnum/tone used for portamento" },

            { name: "volume", size: "db" },
            { name: "midi_note", size: "db" },
            { name: "instrument_num", size: "db" },
            
            { name: "slide_amount", size: "db", comment: "how much to add/subtract per tic" },
            { name: "slide_type", size: "db", comment: "type of slide (up/down/portamento)" },

            { name: "vibrato_depth", size: "db" },
            { name: "vibrato_counter", size: "db" },
            { name: "vibrato_counter_add", size: "db" },

            { name: "arpeggio_pos", size: "db"},
            { name: "arpeggio", size: "db" },

            //{ name: "order_table_ptr", size: "dw", comment: "pointer to the current order"},
            { name: "pattern_ptr", size: "dw", comment: "pointer to the current pattern" },
            { name: "line_wait", size: "db", comment: "wait for this many lines"},
            { name: "tic_wait", size: "db", comment: "wait for this many tics" },
            
            { name: "volume_macro_len", size: "db"},
            { name: "volume_macro_vol", size: "db"},
            { name: "volume_macro_pos", size: "db"},
            { name: "volume_macro_loop", size: "db"},
            { name: "volume_macro_ptr", size: "dw"},

            //{ name: "ex_macro_type", size: "db"},
            //{ name: "ex_macro_len", size: "db"},
            //{ name: "ex_macro_val", size: "db"},
            //{ name: "ex_macro_pos", size: "db"},
            //{ name: "ex_macro_loop", size: "db"},
            //{ name: "ex_macro_ptr", size: "dw"},

            //{ name: "patch", size: "db"},
        ]
    },
    {
        name: "music_state",
        members: [
            { name: "magic_byte", size: "db", comment: "should be 0xba for a valid song" },
            { name: "bank", size: "db", },
            { name: "channel_count", size: "db", },

            { name: "flags", size: "db" },

            //{ name: "master_volume", size: "db" },
            //{ name: "master_volume_fade", size: "db" },

            //{ name: "loop", size: "db", },
            //{ name: "has_chips", size: "db", },
            
            //{ name: "sfx_channel", size: "db", },
            { name: "sfx_subchannel", size: "db", },
                
            //{ name: "time_base", size: "db", },
            { name: "speed_1", size: "db", },
            { name: "speed_2", size: "db", },

            { name: "pattern_length", size: "db", },
            { name: "orders_length", size: "db", },
                
            { name: "instrument_ptrs", size: "dw", },
            { name: "order_ptrs", size: "dw", },

            //{ name: "subtic", size: "db", },
            { name: "tic", size: "db", },
            { name: "line", size: "db", },
            { name: "order", size: "db", },
            
            { name: "order_jump", size: "db" },

            // { name: "process_new_line", size: "db", },
            
            { name: "noise_mode", size: "db", },
            //{ name: "panning", size: "db" },
        ]
    },
];

// output asm file
let out_wladx = fs.openSync("music_driver/banjo_defines_wladx.inc", "w+");
let out_sdas = fs.openSync("music_driver_sdas/banjo_defines_sdas.inc", "w+");

function writelabel(label)
{
    fs.writeSync(outfile, song_prefix + "_" + label + ":" + EOL);
}

// incude guards
fs.writeSync(out_wladx, ".ifndef BANJO_DEFINES\n");
fs.writeSync(out_wladx, ".define BANJO_DEFINES 1\n");

// write out defines
for (let i = 0; i < defines.length; i++)
{
    for (let j = 0; j < defines[i].length; j++)
    {
        let define = defines[i][j];

        // wladx defines
        fs.writeSync(out_wladx, ".define " + define.name + "\t\t" + "0x" + define.value.toString(16) + "\n");

        // sdas defines
        fs.writeSync(out_sdas, define.name + "\t\t\t.equ\t\t" + "0x" + define.value.toString(16) + "\n");
    }

    fs.writeSync(out_wladx, "\n");
    fs.writeSync(out_sdas, "\n");
}

// write out structs
for (let i = 0; i < structs.length; i++)
{
    let struct = structs[i];
    let byte_offset = 0;

    // start wla dx struct
    fs.writeSync(out_wladx, ".STRUCT " + struct.name + "\n");    

    for (let j = 0; j < struct.members.length; j++)
    {
        let member = struct.members[j];
        let comment = member.hasOwnProperty("comment") ? ("\t\t; " + member.comment) : "";

        if (member.size == "db")
        {
            fs.writeSync(out_wladx, "\t" + member.name + ": " + member.size + comment + "\n");
            fs.writeSync(out_sdas, struct.name + "." + member.name + " .equ " + byte_offset + comment + "\n");

            byte_offset += 1;
        }
        else if (member.size == "dw")
        {
            fs.writeSync(out_wladx, "\t" + member.name + ": " + member.size + comment + "\n");
            fs.writeSync(out_sdas, struct.name + "." + member.name + " .equ " + byte_offset + comment + "\n");

            byte_offset += 2;
        }
        else
        {
            fs.writeSync(out_wladx, "\t" + member.name + ": ds " + member.size + comment + "\n");
            fs.writeSync(out_sdas, struct.name + "." + member.name + " .equ " + byte_offset + comment + "\n");

            byte_offset += member.size;
        }
        
    }

    // end wla dx struct
    fs.writeSync(out_wladx, ".ENDST\n");

    fs.writeSync(out_sdas, "_sizeof_" + struct.name + " .equ " + byte_offset + "\n");

    fs.writeSync(out_wladx, "\n");
    fs.writeSync(out_sdas, "\n");
}

fs.writeSync(out_wladx, `

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

`)

// end include guards
fs.writeSync(out_wladx, ".endif\n");

// close files
fs.closeSync(out_wladx);
fs.closeSync(out_sdas);