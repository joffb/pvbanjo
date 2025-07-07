# PVBanjo Sound Driver for Casio PV-1000

### Features:
+ Furnace tracker file conversion
+ WLA-DX support
+ Song and SFX playback

### Furnace features and effects currently supported:
**NB:** *Compatibility Flags > Pitch/Playback > Pitch Linearity* should be set to "None" in Furnace
+ Volume macro per channel (only "sequence" mode)
+ Effects (multiple effects per channel are supported):
    + Arpeggios (00)
    + Pitch slides up and down (01, 02)
    + Portamento (03)
    + Vibrato (04)
    + Set speed 1 (09)
    + Set speed 2 (0F)
    + Jump to pattern (0B)
    + Jump to next pattern (0D)
        + NB: ignores the parameter and goes to the start of the next pattern
    + XOR mode (10)
    + Note cut (EC)
        + NB: delays longer than the current line's speed will not work properly
    + Note delay (ED)
        + NB: delays longer than the current line's speed will not work properly

### furnace2json
This is a script to turn a furnace .fur file into a json representation

`python furnace2json.py -o output.json input.fur`

### json2pv

This is a script to turn the json output of furnace2json into an .asm file.
The -i parameter can be used to determine the name of the song's variable in C or its label in Asm.

For song in a WLA-DX project, you could use it like:

```python json2sms.py -i song_name -o output.asm input.json ```

For sfx you use it with "-s" like:

```python json2sms.py -i sfx_name -s 2 -o output.asm input.json ```

SFX only play back on one channel, specified by the -s parameter. So here "-s 2" the channel number which the sfx is in (zero indexed, so the first channel in the .fur is 0). Other channels will not be included in the export!

## Using the sound driver

The `banjo_defines_wladx.inc` should be included to provide the sizes of `music_state` and `channel` structs used in channel definitions.

### Channel definitions

These variables should be defined somewhere in RAM:

```
	song_playing: db
	song_state: INSTANCEOF music_state
	song_channels: INSTANCEOF channel (3)
	
	sfx_playing: db
	sfx_state: INSTANCEOF music_state
	sfx_channel: INSTANCEOF channel
```

### Initialisation

Use `call banjo_init` to initialise Banjo's variables and the sound chip.

### Playing songs and sfx

Just load the pointer to the song data or SFX data into hl and call the correct function:

```
ld hl, p1_theme
call banjo_play_song


ld hl, paddle_hit_sfx
call banjo_play_sfx

```

Once per frame, you should do `call banjo_update_song` and `call banjo_update_sfx`.

### Looping
By default Songs will loop and SFX will not. The loop mode can be changed with the `banjo_set_song_loop_mode` and `banjo_set_sfx_loop_mode` functions. Passing 0 will switch looping off and anything >=1 will switch looping on.

### Song data
The converted Song/SFX data is similar to the tracker data. Each Channel has an Order array which has an array of pointers to Patterns in the order they're to be played back.

A Pattern for a Channel is an array of bytes which map to commands. The end of a line is signified by a byte with bit 7 set, so anything >= 0x80 (LINE_WAIT). When this is reached it stops processing the channel's commands and waits for a variable number of Lines before processing begins again, with the number of lines to wait defined by the lower 7 bits. A list of commands in the pattern might look like:

`INSTRUMENT_CHANGE, 3, ARPEGGIO, 87, NOTE_ON, 15, (LINE_END | 15), NOTE_OFF`

Which would mean change the channel's instrument to 3, start an arpeggio that goes [0, 5, 7] (87 == 0x57), start playing note number 15, stop processing this line and wait for 15 lines, then stop playing the note.
