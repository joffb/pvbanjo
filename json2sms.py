#!/usr/bin/env python3

# json2sms
# Joe Kennedy 2024

import sys
import json
import math
from pathlib import Path
from optparse import OptionParser

MAGIC_BYTE          = 0xba

NOTE_ON             = 0x00
NOTE_OFF            = 0x01
INSTRUMENT_CHANGE   = 0x02
VOLUME_CHANGE       = 0x03
XOR_MODE_ON         = 0x04
XOR_MODE_OFF        = 0x05
SLIDE_UP            = 0x06
SLIDE_DOWN          = 0x07
SLIDE_PORTA         = 0x08
SLIDE_OFF           = 0x09
ARPEGGIO            = 0x0a
ARPEGGIO_OFF        = 0x0b
VIBRATO             = 0x0c
VIBRATO_OFF         = 0x0d
LEGATO_ON           = 0x0e
LEGATO_OFF          = 0x0f
ORDER_JUMP          = 0x10
SET_SPEED_1         = 0x11
SET_SPEED_2         = 0x12
ORDER_NEXT          = 0x13
NOTE_DELAY          = 0x14
END_LINE            = 0x80

INSTRUMENT_SIZE     = 16
FM_PATCH_SIZE       = 8

CHIP_SN76489        = 0x03      # 0x03: SMS (SN76489) - 4 channels
CHIP_SN76489_OPLL   = 0x43      # 0x43: SMS (SN76489) + OPLL (YM2413) - 13 channels (compound!)
CHIP_OPLL           = 0x89      # 0x89: OPLL (YM2413) - 9 channels
CHIP_OPLL_DRUMS     = 0xa7      # 0xa7: OPLL drums (YM2413) - 11 channels
CHIP_AY_3_8910      = 0x80      # AY-3-8910 - 3 channels
CHIP_PV_1000        = 0xcb

MACRO_TYPE_VOLUME   = 0
MACRO_TYPE_ARP      = 1
MACRO_TYPE_DUTY     = 2
MACRO_TYPE_WAVE     = 3
MACRO_TYPE_PITCH    = 4
MACRO_TYPE_EX1      = 5

macro_type_lookup = {
    MACRO_TYPE_VOLUME: 0x02,
    MACRO_TYPE_ARP: 0x40,
    MACRO_TYPE_DUTY: 0x20,
    MACRO_TYPE_WAVE: 0x10,
    MACRO_TYPE_PITCH: 0x01,
    MACRO_TYPE_EX1: 0
}

# whether the drum channel needs its volume to be pre-shifted
pre_shift_fm_drum_value = [0,0,4,0,4]

def ay_pitch_fnum(note, octave):
    
    freq = pow(2.0, ((note + ((octave + 2) * 12) - 69.0)/12.0)) * 440.0
    fnum = 3579545.0 / (2 * 8 * freq)

    return int(fnum)

def ay_envelope_period(fnum, numerator, denominator):

    # calculate envelope period from the fnum
    period = ((fnum * denominator) / numerator) / 16
    period = int(period)

    # make sure period is a valid value
    if period > 0xffff:
        period = 0xffff
    elif period < 1:
        period = 1

    return period


def int_def(v, d):
    try:
        return int(v)
    except ValueError:
        return d

def main(argv=None):
    print("json2sms - Joe Kennedy 2024")

    parser = OptionParser("Usage: json2sms.py [options] INPUT_FILE_NAME.JSON")
    parser.add_option("-o", '--out',        dest='outfilename',                                      help='output file name')
    parser.add_option("-i", '--identifier', dest='identifier',                                       help='source identifier')
    parser.add_option("-s", "--sfx",        dest='sfx',                               default="-1",  help='SFX channel')

    parser.add_option("-b", '--bank',       dest='bank',                                             help='BANK number')
    parser.add_option("-a", '--area',       dest='area',                                             help='AREA name')

    parser.add_option("", '--sdas',       dest='sdas',                                action="store_true",    help='SDAS assembler compatible output')

    (options, args) = parser.parse_args()

    if (len(args) == 0):
        parser.print_help()
        parser.error("Input file name required\n")

    infilename = Path(args[0])

    sfx = (int_def(options.sfx, -1) != -1)
    sfx_channel = int_def(options.sfx, 0) if sfx else 0

    in_filename = Path(args[0])
    out_filename = Path(options.outfilename) if (options.outfilename) else infilename.with_suffix('.c')
    song_prefix = options.identifier if (options.identifier) else str(Path(infilename.name).with_suffix(''))

    # try to load json file
    try:
        infile = open(str(in_filename), "r")
        # read into variable
        json_text = infile.read()
        infile.close()
    except OSError:
        print("Error reading input file: " + str(in_filename), file=sys.stderr)
        sys.exit(1)

    # try to parse json
    try:
        song = json.loads(json_text)
    except ValueError:
        print("File is not a valid json file: " + in_filename, file=sys.stderr)
        sys.exit(1)

    pv_1000_found = False

    for sound_chip in song['sound_chips']:
        if (sound_chip == CHIP_PV_1000):    
            pv_1000_found = True

    if pv_1000_found == False:

        print("File does not contain PV-1000 chip data", file=sys.stderr)
        sys.exit(1)

    # process instruments
    instruments = []

    macros = {}
    fm_patches = {}

    for i in range(0, len(song['instruments'])):

        instrument = song['instruments'][i]

        new_instrument = {
            "volume_macro_len": 0,
            "volume_macro_loop": 0xff,
            "volume_macro_ptr": 0xffff,

            "ex_macro_type": 0,
            "ex_macro_len": 0,
            "ex_macro_loop": 0xff,
            "ex_macro_ptr": 0xffff,

            "fm_patch": 0xff,
            "fm_patch_ptr": 0xffff,
        }

        # macros
        for macro in instrument["macros"]:

            if macro["loop"] != 0xff and macro["loop"] >= macro["length"]:
                macro["loop"] = 0xff

            # volume macro
            if macro["code"] == MACRO_TYPE_VOLUME:

                new_instrument["volume_macro_len"] = macro["length"]
                new_instrument["volume_macro_loop"] = macro["loop"]

                for j in range(0, macro["length"]):                     
                    macro['data'][j] = 1 if macro['data'][j] > 0 else 0

                macro_data_name = "macro_volume_" + str(i)
                macros[macro_data_name] = macro['data']
                new_instrument["volume_macro_ptr"] = macro_data_name

            # currently no ex macro for this instrument
            elif new_instrument["ex_macro_ptr"] == 0xffff:

                new_instrument["ex_macro_type"] = macro_type_lookup[macro["code"]] if macro["code"] in macro_type_lookup else macro["code"]
                new_instrument["ex_macro_len"] = macro["length"]
                new_instrument["ex_macro_loop"] = macro["loop"]

                if macro["code"] == MACRO_TYPE_ARP:
                    for j in range(0, macro["length"]):
                        val = macro['data'][j]

                        # get the absolute/relative flag in bit 7
                        absolute = ((abs(val) >> 23) & 0x80)
                        
                        # constrain the arp amount to 7 bits
                        macro['data'][j] = (macro['data'][j] & 0x7f) | absolute

                # need to invert pitch offset for certain chips
                if macro["code"] == MACRO_TYPE_PITCH and (instrument["type"] == 0 or instrument["type"] == 6):
                    for j in range(0, macro["length"]):
                        macro['data'][j] = -macro['data'][j]

                # need to pre-shift fm patch for opll patch macro
                if macro["code"] == MACRO_TYPE_WAVE and instrument["type"] == 13:
                    for j in range(0, macro["length"]):
                        macro['data'][j] = macro['data'][j] << 4

                # need to invert duty for ay noise freq macro
                if macro["code"] == MACRO_TYPE_DUTY and instrument["type"] == 6:
                    for j in range(0, macro["length"]):
                        macro['data'][j] = 0x1f - macro['data'][j]

                # special case for sn noise duty, similar to SN_NOISE_MODE command
                if macro["code"] == MACRO_TYPE_DUTY and instrument["type"] == 0:
                    for j in range(0, macro["length"]):
                        duty = macro['data'][j]
                        noise_mode = (duty & 0x1) << 2
                        noise_freq = 0x3 if (duty & 0x2) else 0x0
                        macro['data'][j] = 0x80 | (0x3 << 5) | noise_mode | noise_freq

                macro_data_name = "macro_ex_" + str(i)
                macros[macro_data_name] = macro['data']
                new_instrument["ex_macro_ptr"] = macro_data_name

        instruments.append(new_instrument)

    # process patterns
    patterns = {}

    # process patterns
    # go through each channel
    for i in range(0, song['channel_count']):

        channel_subchannel = i
        
        # separate pattern arrays per channel
        patterns[i] = {}

        # go through each pattern in the channel
        for j in range (0, len(song['patterns'][i])):

            # keep track of the current instrument
            # so we don't have multiple INSTRUMENT_CHANGEs per note
            # when the instrument is exactly the same
            # reset at a pattern level as patterns may appear out of order
            # or be jumped to
            current_inst = -1
            current_vol = -1

            pattern = song['patterns'][i][j]

            pattern_bin = []

            if (pattern):

                last_note = { "note": -1, "octave": -1 }
                ay_envelope_auto = False

                # go through every line
                for k in range (0, song['pattern_length']):

                    line = pattern['data'][k]

                    note = { "note": line['note'], "octave": line['octave'] }

                    # instrument has changed
                    if (current_inst != line['instrument'] and line['instrument'] != -1):

                        current_inst = line['instrument']

                        # ensure the instrument exists so we don't load
                        # other data as an instrument
                        if current_inst < len(instruments):

                            pattern_bin.append(INSTRUMENT_CHANGE)
                            pattern_bin.append(current_inst)

                    # check effects
                    for eff in range (0, len(line['effects']), 2):

                        # xor mode
                        if (line['effects'][eff] == 0x10):

                            if (line['effects'][eff + 1] > 0):

                                pattern_bin.append(XOR_MODE_ON)

                            else:

                                pattern_bin.append(XOR_MODE_OFF)

                        # Arpeggio
                        elif (line['effects'][eff] == 0x00):

                            if (line['effects'][eff + 1] != 0):
                                pattern_bin.append(ARPEGGIO)
                                pattern_bin.append(line['effects'][eff + 1])
                            else:
                                pattern_bin.append(ARPEGGIO_OFF)


                        # Pitch slide up
                        elif (line['effects'][eff] == 0x01):

                            if (line['effects'][eff + 1] != 0):
                                pattern_bin.append(SLIDE_UP)
                                pattern_bin.append(line['effects'][eff + 1])
                            else:
                                pattern_bin.append(SLIDE_OFF)


                        # Pitch slide down
                        elif (line['effects'][eff] == 0x02):

                            if (line['effects'][eff + 1] != 0):
                                pattern_bin.append(SLIDE_DOWN)
                                pattern_bin.append(line['effects'][eff + 1])
                            else:
                                pattern_bin.append(SLIDE_OFF)


                        # Portamento
                        elif (line['effects'][eff] == 0x03):

                            if (line['effects'][eff + 1] != 0):
                                pattern_bin.append(SLIDE_PORTA)
                                pattern_bin.append(line['effects'][eff + 1])
                            else:
                                pattern_bin.append(SLIDE_OFF)

                        # Vibrato
                        elif (line['effects'][eff] == 0x04):

                            vibrato_speed = line['effects'][eff + 1] >> 4
                            vibrato_amount = line['effects'][eff + 1] & 0xf

                            if (vibrato_speed == 0 and vibrato_amount == 0):
                                pattern_bin.append(VIBRATO_OFF)

                            else:
                                pattern_bin.append(VIBRATO)
                                pattern_bin.append(vibrato_speed)
                                pattern_bin.append(vibrato_amount << 4)

                        # legato
                        elif (line['effects'][eff] == 0xea):

                            if (line['effects'][eff + 1] > 0):

                                pattern_bin.append(LEGATO_ON)

                            else:

                                pattern_bin.append(LEGATO_OFF)

                        # order jump
                        elif (line['effects'][eff] == 0x0b):
                            
                            pattern_bin.append(ORDER_JUMP)
                            pattern_bin.append(line['effects'][eff + 1])

                        # order next
                        elif (line['effects'][eff] == 0x0d):
                            
                            pattern_bin.append(ORDER_NEXT)

                        # set speed 1
                        elif (line['effects'][eff] == 0x09):
                            
                            pattern_bin.append(SET_SPEED_1)
                            pattern_bin.append(line['effects'][eff + 1] * (song['time_base'] + 1))

                        # set speed 2
                        elif (line['effects'][eff] == 0x0f):
                            
                            pattern_bin.append(SET_SPEED_2)
                            pattern_bin.append(line['effects'][eff + 1] * (song['time_base'] + 1))

                        # note delay
                        elif (line['effects'][eff] == 0xed):

                            pattern_bin.append(NOTE_DELAY)
                            pattern_bin.append(line['effects'][eff + 1] * (song['time_base'] + 1))

                    # volume
                    # if the volume has been specified on the line, or we haven't provided a volume command yet
                    if (line['volume'] != -1):

                        if (line['volume'] != -1):
                            volume = line['volume']

                        # volume has changed
                        if volume != current_vol:

                            pattern_bin.append(VOLUME_CHANGE)
                            pattern_bin.append(volume)

                        current_vol = volume

                    # empty
                    if (line['note'] == -1 and line['octave'] == -1):

                        False

                    # note on
                    elif (line['note'] >= 0 and line['note'] < 12):

                        pattern_bin.append(NOTE_ON)

                        # align the note octave with the pitch table in rom
                        octave = (note['octave'] - 3)

                        # ensure the octave/note combo is valid
                        # for the pitch table in rom
                        if octave < 0:
                            octave = 0
                        elif octave > 5:
                            octave = 5
                            note['note'] = 11 

                        # note number
                        pattern_bin.append((note['note'] + (octave * 12)) & 0x7f)

                        # store in last_note
                        last_note = note


                    # note off
                    elif (line['note'] == 100 or line['note'] == 101):

                        pattern_bin.append(NOTE_OFF)

                    # special case for note cut
                    for eff in range (0, len(line['effects']), 2):
                    
                        # note cut
                        if (line['effects'][eff] == 0xec):

                            pattern_bin.append(NOTE_DELAY)
                            pattern_bin.append(line['effects'][eff + 1] * (song['time_base'] + 1))
                            pattern_bin.append(NOTE_OFF)

                    # end line marker
                    pattern_bin.append(END_LINE)


                # when a run of END_LINE ends, replace it if it's long enough
                def run_end(run_length, pattern_bin_rle):

                    if (run_length > 0):
                        pattern_bin_rle.append("(" + str(END_LINE) + " | " + str(run_length - 1) + ")")


                # run length encoding for END_LINE
                run_length = 0
                pattern_bin_rle = []


                # look through pattern_bin for runs of END_LINE
                for k in range(0, len(pattern_bin)):

                    if (pattern_bin[k] == END_LINE):

                        run_length += 1

                    else:

                        run_end(run_length, pattern_bin_rle)
                        run_length = 0
                        pattern_bin_rle.append(pattern_bin[k])

                run_end(run_length, pattern_bin_rle)

                pattern_bin = pattern_bin_rle

                # add to patterns array
                patterns[i][pattern["index"]] = pattern_bin

    # output asm/h file
    outfile = open(str(out_filename), "w")

    def writelabel(label):

        outfile.write(song_prefix + "_" + label + ":" + "\n")

    bank_number = str(options.bank) if (options.bank) else "0"

    # sdas/sdcc mode
    if (options.sdas):

        # prefix song name with an underscore so it's accessible in C
        song_prefix = "_" + song_prefix

        # prefix asm calls song name with an underscore
        channel_init_calls = list(map(lambda call: "_" + call, channel_init_calls))
        channel_update_calls = list(map(lambda call: "_" + call, channel_update_calls))
        channel_mute_calls = list(map(lambda call: "_" + call, channel_mute_calls))
        song_mute_calls = list(map(lambda call: "_" + call, song_mute_calls))

        # set the module for this file so it flags up the file name in any errors
        outfile.write(".module " + song_prefix + "\n")

        # export the song label so it's accessible in C
        outfile.write(".globl " + song_prefix + "\n")

        # area specified
        if (options.area):

            # postfix the area name with the bank number if available
            outfile.write(".area _" + options.area + (str(options.bank) if options.bank else "") + "\n")

        # bank specified
        if (options.bank):
            
            # used for gbdk autobanking
            outfile.write("___bank" + song_prefix + " .equ " + str(options.bank) + "\n")
            outfile.write(".globl ___bank" + song_prefix + "\n")
            bank_number = "___bank" + song_prefix

        outfile.write("\n")

    if sfx:
        sfx_subchannel = sfx_channel


    # initial label
    outfile.write(song_prefix + ":" + "\n")

    # basic song data
    writelabel("magic_byte")
    outfile.write(".db " + str(MAGIC_BYTE) + "\n")
    writelabel("bank")
    outfile.write(".db " + bank_number + "\n")
    writelabel("channel_count")
    outfile.write(".db " + str(1 if sfx else song['channel_count']) + "\n")
    writelabel("flags")
    outfile.write(".db " + str(8 if sfx else 2) + "\n")
    #writelabel("master_volume")
    #outfile.write(".db 0x80\n")
    #writelabel("master_volume_fade")
    #outfile.write(".db 0x00\n")
    #writelabel("has_chips")
    #outfile.write(".db " + str(song['has_chips']) + "\n")
    #writelabel("sfx_channel")
    #outfile.write(".db " + str(sfx_channel if sfx else 0xff) + "\n")
    writelabel("sfx_subchannel")
    outfile.write(".db " + str(sfx_subchannel if sfx else 0xff) + "\n")
    #writelabel("time_base")
    #outfile.write(".db " + str(song['time_base'] + 1) + "\n")
    writelabel("speed_1")
    outfile.write(".db " + str(song['speed_1'] * (song['time_base'] + 1)) + "\n")
    writelabel("speed_2")
    outfile.write(".db " + str(song['speed_2'] * (song['time_base'] + 1)) + "\n")
    writelabel("pattern_length")
    outfile.write(".db " + str(song['pattern_length']) + "\n")
    writelabel("orders_length")
    outfile.write(".db " + str(song['orders_length']) + "\n")
    writelabel("instrument_ptrs")
    outfile.write(".dw " + song_prefix + "_instrument_data" + "\n")
    writelabel("order_ptrs")
    outfile.write(".dw " + song_prefix + "_orders" + "\n")
    #writelabel("subtic")
    #outfile.write(".db 0\n")
    writelabel("tic")
    outfile.write(".db " + str(song['speed_1'] * (song['time_base'] + 1)) + "\n")
    writelabel("line")
    outfile.write(".db " + str(song['pattern_length']) + "\n")
    writelabel("order")
    outfile.write(".db 0\n")
    writelabel("order_jump")
    outfile.write(".db 0xff\n")
    writelabel("noise_mode")
    outfile.write(".db 0\n")
    writelabel("panning")
    outfile.write(".db 0xff\n")

    outfile.write("\n" + "\n")

    # macros
    writelabel("macros")

    for key in macros:

        writelabel(key)
        outfile.write(".db " + ", ".join(map(str, macros[key])) + "\n")

    outfile.write("\n" + "\n")

    # instruments
    #writelabel("instrument_pointers")

    #for i in range (0, len(instruments)):

    #    outfile.write(".dw " + song_prefix + "_instrument_" + str(i) + "\n")

    #outfile.write("\n" + "\n")

    # instruments
    writelabel("instrument_data")

    for i in range (0, len(instruments)):

        instrument = instruments[i]

        writelabel("instrument_" + str(i))
        outfile.write("\t.db " + str(instrument["volume_macro_len"]) + " ; len\n")
        outfile.write("\t.db " + str(instrument["volume_macro_loop"]) + " ; loop\n")

        if (instrument["volume_macro_ptr"] == 0xffff):
            outfile.write("\t.dw 0xffff ;ptr\n")
        else:
            outfile.write("\t.dw " + song_prefix + "_" + str(instrument["volume_macro_ptr"]) + " ; ptr\n")

        # ex macro
        #outfile.write("\t.db " + str(instrument["ex_macro_type"]) + " ; type\n")
        #outfile.write("\t.db " + str(instrument["ex_macro_len"]) + " ; len \n")
        #outfile.write("\t.db " + str(instrument["ex_macro_loop"]) + " ; loop\n")

        #if (instrument["ex_macro_ptr"] == 0xffff):
        #    outfile.write("\t.dw 0xffff ;ptr\n")
        #else:
        #    outfile.write("\t.dw " + song_prefix + "_" + str(instrument["ex_macro_ptr"]) + " ; ptr\n")

        # fm preset
        #outfile.write("\t.db " + str(instrument["fm_patch"]) + "\n")

        # fm patch
        #if (instrument["fm_patch_ptr"] == 0xffff):
        #    outfile.write("\t.dw 0xffff ;ptr\n")
        #else:
        #    outfile.write("\t.dw " + song_prefix + "_" + str(instrument["fm_patch_ptr"]) + " ; ptr\n")

    outfile.write("\n" + "\n")

    # orders
    writelabel("orders")

    for j in range (0, len(song['orders'][i])):

        outfile.write("\t.dw ")

        for i in range (0, song['channel_count']):

            if sfx and sfx_channel != i:
                continue

            outfile.write(song_prefix + "_patterns_" + str(i) + "_" + str(song['orders'][i][j]))
            outfile.write(", ")


        outfile.write("\n")


    outfile.write("\n" + "\n")

    # patterns
    writelabel("patterns")

    for i in range (0, song['channel_count']):

        if sfx and sfx_channel != i:
            continue

        for j in range (0, len(song["patterns"][i])):

            pattern_index = song["patterns"][i][j]["index"]

            writelabel("patterns_" + str(i) + "_" + str(pattern_index))
            outfile.write(".db " + ", ".join(map(str, patterns[i][pattern_index])) + "\n")

    #outfile.write("\n\n.rept 10000\n.db 0xba\n.endm\n")

    # close file
    outfile.close()

if __name__=='__main__':
    main()
