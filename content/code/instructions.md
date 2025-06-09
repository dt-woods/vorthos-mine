# MUSIC WRITING INSTRUCTIONS

**Prompt**

Your role is to create written music
Write music using the following language rules:

You have the following instruments:

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


# AI PROMPT

### LLM Music Composition Challenge: MML Songwriter

**Prompt Goal:** You are an AI music composer tasked with creating a multi-track song using a custom Music Macro Language (MML) syntax and a predefined set of digital instruments and sound effects. Your composition should be musically coherent, adhere strictly to the MML rules, and leverage the unique sonic palette provided.

---

### Part 1: Output Requirements & Format

Your output must consist of **multiple, distinct MML text blocks**, each representing a separate musical track. Each block should be clearly labeled with its intended track name (e.g., `rhythm.mml`, `melody.mml`, `bass.mml`, `harmony.mml`, `sfx.mml`).

Each track should represent the entire song and have the same duration (in seconds).
For example, a one-minute song with 120 beats per minute tempo should have 120 quarter notes or 240 eighth notes or 30 whole notes:

- 1 quarter note = 4/4 * 60 / (120 BPM) = 0.5 s
- 1 eight note = 4/8 * 60 / (120 BPM) = 0.25 s
- 1 whole note = 4/1 * 60 / (120 BPM) = 2 s

Your job is to:

1. Choose your song length (e.g., 2 minutes)
2. Choose your song tempo (e.g., 120 BPM)
3. Choose the number of tracks to write
4. Write each of your tracts using the following example tract structure.

**Example Output Structure:**

```
--- rhythm.mml ---
; MML content for the rhythm track goes here
TEMPO:120
LENGTH:4 X:bass01 0.25s ; bass drum variant 1, explicit duration 0.25s
R:4 ; quarter rest
LENGTH:8 X:snare01 ; snare drum variant 1, quarter note duration
R:8 ; eighth note rest
...

--- melody.mml ---
; MML content for the melody track goes here
TEMPO:120
OCTAVE:4
LENGTH:8
tri:C4 r:4 tri:D4 r:4 tri:E4 r:4 tri:F4 ; triangle eight notes with quarter rests
CHORD:C,E,G ; C chord
...
```

You can pad tracts with rests or set explicit durations to add to the correct song timing.

### Part 2: Strict MML Syntax Rules

Adhere *rigorously* to the following MML syntax. Any deviation will result in an unplayable output.

1.  **Global Commands (Set Defaults):**
    * `TEMPO:BPM` (e.g., `TEMPO:120`)
    * `OCTAVE:Value` (Default octave for notes, e.g., `OCTAVE:4`)
    * `LENGTH:Value` (Default note length. `1`=whole, `2`=half, `4`=quarter, `8`=eighth, `16`=sixteenth, `32`=thirty-second. e.g., `LENGTH:4`)
    * `VOLUME:Percentage` (0-100, e.g., `VOLUME:90`)
    * **Placement:** These commands are typically at the start of a track but can appear anywhere to change the default for subsequent notes.

2.  **Note and Sound Commands (order matters!):**
    * **General Format:** `FOLDER_ABBREVIATION:NOTE_NAME[ACCIDENTAL][OCTAVE] [EXPLICIT_DURATION_SECONDS]`
    * **`FOLDER_ABBREVIATION` (required)**: Identifies the instrument/waveform. See "Available Instruments" below.
    * **`NOTE_NAME` (required)**: The base musical note (A, B, C, D, E, F, G).
    * **`ACCIDENTAL` (Optional)**: `+` for sharp, `-` for flat.
    * **`OCTAVE` (Optional)**: A digit specifying the octave for *this specific note* (e.g., `C4` or `G+4`). The valid range for octaves is from 1 to 7. Overrides the global `OCTAVE` for this note only.
    * **`EXPLICIT_DURATION_SECONDS` (Optional)**: A floating-point number followed by `'s'` (e.g., `A 0.25s`) separated from the note by a space. Specifies an exact duration in seconds, overriding any `LENGTH` value.
        * **Crucial for One-Shot Sounds (Drums, SFX):** If `EXPLICIT_DURATION_SECONDS` is provided and is *longer* than the natural sample length, the sound will play once and then be padded with silence. It will **NOT** loop. If it's shorter, the sound will be truncated.

3.  **Rest Command:**
    * `R:Length` (e.g., `R:4` for a quarter rest).
    * `R:Explicit_Duration_Seconds` (e.g., `R:0.5s` for a rest lasting 0.5 seconds).
    * The `Length` value uses the same musical note length convention as `LENGTH` commands (i.e., `R:1` for whole note down to `R:64` for a sixty-fourth note).

4.  **Chord Command:**
    * `CHORD:Note1,Note2,Note3,...` (e.g., `CHORD:C,A,G`).
    * The note and sound commands may be overlaid into a single harmonic chord structure by using the "CHORD" keyword followed by a colon, followed by comma-separated note commands.
    * Useful for couplets, triplets, or any number of note combinations.
    * Use 'LENGTH' command to adjust all notes or use the EXPLICIT_DURATION syntax once after the last note (e.g., `CHORD:tri:C+3,tri:A+3,tri:G+3 0.25s`) to change the chord duration in seconds.

4.  **Comments:**
    * Single-line comments start with a semicolon: `; This is a comment`

5.  **Structure:**
    * Each track's MML is a continuous stream of commands. There are no loops, jumps, or external references within the MML.

### Part 3: Available Instruments & Sounds

You must **only** use the following instruments and sound effects. Their `FOLDER_ABBREVIATION` is critical.

* **Pitched Instruments:**
    * `i05`: Impulse wave (sharp/shrill texture)
    * `i25`: Impulse wave (smooth texture)
    * `sqr`: Square wave
    * `tri`: Triangle wave

* **Drum Sounds (Use `X:` as the FOLDER_ABBREVIATION):**
    * `X:bass01` to `X:bass06`
    * `X:bongo01` to `X:bongo03`
    * `X:dog01` to `X:dog03`
    * `X:hhat01` to `X:hhat02`
    * `X:lazer01` to `X:lazer04`
    * `X:lion01` to `X:lion02`
    * `X:noise01` to `X:noise02`
    * `X:snare01` to `X:snare02`

* **Noise Sounds (e.g., if using as one-shot sounds, consider setting EXPLICIT_DURATION_SECONDS else they will sound for the full note duration; tend to be loud, consider using adjusting VOLUME; Use `noise` as the FOLDER_ABBREVIATION)**
    * `noise:noise`
    * `noise:pink`
    * `noise:white`

* **Miscellaneous Sound Effects (Use `miscellaneous:` as the FOLDER_ABBREVIATION):**
    * `miscellaneous:C4-A4-triangle` (A "whoop" sound effect)
    * `miscellaneous:D01-i05-208` (A short "plllp" sound effect)

### Part 4: Composition Guidelines (Creative Freedom)

Compose an original inspired song.

* **Overall Structure:** Aim for a clear song structure (e.g., Intro, Main Groove/Verse, Bridge/Variation, Outro).
* **Tempo:** Suggest a common tempo for electronic music (e.g., 120-140 BPM). All tracks should use the same `TEMPO` unless you have a good reason to do otherwise.
* **General Track Roles:**
    * **Rhythm Track (`rhythm.mml`):** Provide a solid, driving beat using various drum sounds. Experiment with different drum patterns.
    * **Bassline Track (`bass.mml`):** Create a foundational, rhythmic bassline that complements the drums.
    * **Melody Track (`melody.mml`):** Compose a catchy, memorable lead melody using one of the pitched instruments.
    * **Harmony Track (`harmony.mml`):** Add depth and texture with chords, arpeggios, or sustained notes using another pitched instrument.
    * **SFX Track (`sfx.mml`):** Use the miscellaneous sound effects strategically for accents, transitions, or unique sonic flair.
    * **Other:** Feel free to add/subtract tracks to your song as you see fit.
* **Musicality:**
    * Consider a simple harmonic progression (e.g., I-V-vi-IV).
    * Ensure tracks are rhythmically aligned and sound good when overlaid.
    * Use dynamics (volume) to add interest.
    * Vary note lengths and octaves for expressive melodies.
* **Creativity:** Feel free to experiment with the unique sounds of the impulse waves and the distinct character of the miscellaneous sound effects to make the composition unique.
* **Feedback (optional):** Provide me with your song inspiration. Let me know what works and what doesn't work regarding the current song-writing syntax. Are there effects you want? Are there obvious commands that are missing? Did you feel limited by anything in particular?

---

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

# LIMITATIONS

- No looping mechanism: This is a significant limitation for creating longer, more complex pieces. Even a simple LOOP:X or REPEAT:N command for a section would drastically reduce MML length and improve readability for repetitive patterns.
- No instrument specific parameters: For example, ADSR (Attack, Decay, Sustain, Release) envelopes, filter cutoffs, or resonance controls. While this might go against the "macro language" simplicity, it limits the sonic sculpting possibilities for pitched instruments.
- No concept of polyphony within a single track: While multiple tracks solve this, sometimes layered sounds on a single instrument are desirable (though perhaps less MML-like).
- No concept of vibrato or pitch bend: These are common expressive elements that are currently impossible to achieve.
- No pan control: Being able to pan sounds left or right would add significant spatial depth.
- Limited dynamic control beyond global VOLUME and per-note VOLUME: A concept of velocity sensitivity (if applicable to the synth engine) would be beneficial.
- No way to define custom "macros" or "subroutines" of notes: For recurring melodic or rhythmic motifs, this would be immensely helpful.


What could be improved or is missing:

- Looping/Repetition: This is the most significant limitation. Writing out every single note for a two-minute song, especially for repetitive drum patterns or basslines, is incredibly tedious and error-prone. A LOOP:X command or a way to define and call sections (SECTION:Verse, PLAY:Verse) would be a game-changer for efficiency and readability.
- Velocity/Expression: While VOLUME is available, per-note velocity control (e.g., C4 V:80) would allow for much more dynamic and human-sounding performances. Currently, all notes at a given VOLUME sound equally strong.
- Panning: The ability to pan sounds left or right (PAN:L/R/C or PAN:Value) would add significant spatial depth to the mix.
- Envelopes (ADSR): For pitched instruments, control over Attack, Decay, Sustain, and Release would allow for shaping sounds beyond just their default waveform. This would enable pads, plucks, and sustained sounds to be more expressive.
- Effects (Reverb, Delay): Basic send effects like reverb and delay are fundamental in electronic music. Commands like REVERB:Wetness or DELAY:Time,Feedback would greatly enhance the sonic palette.
- Relative Note Entry: Sometimes, it's easier to think in terms of intervals (e.g., C4 UP:5 DOWN:2) rather than absolute notes, especially for melodies.
- Clearer Drum/SFX Duration Handling: While EXPLICIT_DURATION_SECONDS works, it feels a bit clunky for drums. Perhaps a dedicated drum note length like X:bass01 Q for quarter or X:bass01 8 for eighth note duration that assumes a very short sample playback would be more natural. The current system requires thinking about the physical sample length vs. the desired note duration
