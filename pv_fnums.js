/*
	Joe Kennedy 2024
	Create asm include of SN76489 fnums
*/

const fs = require('fs');	 
const note_names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];

// fsquare = fASIC ÷ 1024 ÷ (63-P) ≈ 8739 Hz ÷ (63-P)
// (63-P) = (8739) / fsquare
// P = 63 - (8739) / fsquare
// Where fASIC is the 17.897727MHz crystal divided by 2. 

const MUSIC_CLOCK = 8739.12451171875;

// midi note frequencies for all midi notes

var midi_note_frequencies = [];

for (var i = 0; i < 128; i++)
{
	midi_note_frequencies.push(Math.pow(2.0, (i - 49.0)/12.0) * 138.7162621);
}


// fnum values for the sn chip for the given frequencies

var sn_fnums = [];

for (var i = 0; i < 128; i++)
{
	var freq = 63 - (MUSIC_CLOCK / midi_note_frequencies[i]);

	sn_fnums.push(freq > 62 ? 62 : (freq < 0 ? 0 : freq));
}

var output = [];

for (var i = 48; i < 120; i++)
{
	var out;
	var note_name = note_names[i % 12];
	note_name = "midi: " + note_name + (Math.floor(i / 12) - 1) + ", fur: " + note_name + (Math.floor(i / 12) - 3);
	
	output.push({fnum: Math.round(sn_fnums[i]) & 63, note_name: note_name});
}

// output an include file with the pv note data
let outfile = fs.openSync("music_driver/fnums_pv.inc", "w+");

fs.writeSync(outfile, "pv_tone_lookup:\n");

for (var i = 0; i < output.length; i++)
{
	fs.writeSync(outfile, "; " + output[i].note_name + "\n");
	fs.writeSync(outfile, ".db " + output[i].fnum + "\n");
}

fs.closeSync(outfile);	   
