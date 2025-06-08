# MUSIC WRITING INSTRUCTIONS

**Prompt**

Your role is to create written music
Write music using the following language rules:

You have the following instruments, which work well for notes < 1s in length:

- 'i05', impulse wave (sharp/shrill)
- 'i25', impulse wave (smooth)
- 'sqr', square wave
- 'tri', triangle wave

The instruments have keys: A-G, sharp (+) and flat (-), with octaves that range from 1 to 7 ('o1' to 'o7').

There are three noise instruments: noise, pink, and white.
These instruments have not pitch, nor any octave.

There are drums of various types and variants, including:

- bass, variants 01-06
- bongo, variants 01-03
- dog, variants 01-03
- hhat, variants 01-02
- lazer, variants 01-04
- lion, variants 01-02
- noise, variants 01-02
- snare, variants 01-02

Lastly, there are a few miscellaneous sound effects:

- BASS, variants 01-04 (two-beat phaser sound)
- C4-A4-triangle (a whoop sound effect)
- D01-i05-208 (a short plllp sound effect)

There are five command keywords you can use:

- 'TEMPO': use this keyword with a colon and integer to change the song's tempo in beats per minute (e.g., TEMPO:120 sets 120 BPM).
- VOLUME: use this keyword followed by a colon and an integer to change the   song's current volume, adjustable from zero to 100 (e.g., VOLUME:50).
- 'OCTAVE': use this keyword to set the song's default octave (e.g., OCTAVE:1 or OCTAVE:7); you can always explicitly give an octave with its note.
- 'LENGTH': use this keyword followed by a colon and an integer to set the   default note length (e.g., LENGTH:4 for quarter note, LENGTH:8 for eighth note); you can always explicitly set the length with its note.
- 'CHORD': use this keyword followed by any number of comma-separated note   commands to create chords (e.g., CHORD:C4,E4,G4 for a standard C chord in   squarewave [default instrument] in octave 4 [default octave]).

Notes are defined using the "instrument" : "note" "accidental" "length" "octave" syntax.
For example, "sqr:A-4o3" is a square wave Ab quarter note in octave 3, and "tri:C+1o2" is a triangle wave C# whole note in octave 2.
Percussion is given by "X" followed by a colon and its type and variant.
For example, "X:bongo01" is the bongo drum, variant 01 and "X:bass03" is the bass drum, variant 03.
Miscellaneous effects are given by the "miscellaneous" followed by a colon and its effect name.
For example, "miscellaneous:BASS01" to produce the BASS01 sound effect.
Noise sound effects are accessed using "noise" followed by a colon followed by the type (e.g., "noise:white").
You can provide explicit duration in seconds using a space (e.g., "noise:white 0.5s" produces 1/2 second of white noise).


# NOTES
A quarter note (length 4) is considered one "beat" at the given tempo, $T$ (in beats per minute).
Duration, $D$, of one beat in seconds is 60 divided by $T$.
A whole note (length 1) is 4 beats.
So, the ratio, $R$, for a given note length, $L_{n}$, is (4.0 / length) and:

- for length 4 (quarter note), ratio is 4.0/4 = 1
- for length 8 (eighth note), ratio is 4.0/8 = 0.5
- for length 1 (whole note), ratio is 4.0/1 = 4

Therefore, note length in seconds, $L_{s}$, is duration times ratio, given by the following:

$$
L_{s} = D \times R
$$

where:

$$
D = \frac{60.0}{T}
R = \frac{4}{L_{n}}
$$

_Example_:

17 quarter notes x 1 x 60/120 = 8.5 s
(every note is 0.5 seconds)