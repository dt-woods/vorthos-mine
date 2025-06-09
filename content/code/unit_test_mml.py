import re

# --- rhythm.mml content (copy-pasted from your prompt) ---
rhythm_mml = """
TEMPO:120
VOLUME:90
; Song Length: 2 minutes (240 quarter notes)

; Intro - 4 bars (8 quarter notes) - Total 8 QNs
LENGTH:4
X:bass01 R:4 X:bass01 R:4 X:bass01 R:4 X:bass01 R:4
X:snare01 R:4 X:snare01 R:4 X:snare01 R:4 X:snare01 R:4

; Main Groove (Verse 1 & 2) - 16 bars (32 quarter notes per verse, total 64 QNs)
; Each line represents 1 bar (2 quarter notes / 8 eighth notes)
LENGTH:8
; Verse 1 (8 bars = 32 QNs)
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
; Verse 2 (8 bars = 32 QNs)
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8

; Bridge - 8 bars (16 quarter notes per 4 bars, total 32 QNs)
; More active hi-hats and snares
LENGTH:16
X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:snare02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:hhat02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02
X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:snare02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:hhat02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02
X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:snare02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:hhat02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02
X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:snare02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:hhat02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02
X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:snare02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:hhat02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02
X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:snare02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:hhat02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02
X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:snare02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:hhat02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02
X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:bass03 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:snare02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02 R:16 X:hhat02 R:16 X:hhat02 X:hhat02 X:hhat02 X:hhat02

; Main Groove (Verse 3) - 8 bars (32 quarter notes)
LENGTH:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8
X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:bass01 R:8 X:hhat01 R:8 X:snare01 R:8 X:hhat01 R:8

; Outro - 4 bars (8 quarter notes) - Total 8 QNs
LENGTH:4
X:bass01 R:4 X:snare01 R:4 X:hhat01 R:4 X:hhat01 R:4
X:bass01 R:4 X:snare01 R:4 X:hhat01 R:4 X:hhat01 R:4
X:bass01 R:4 X:snare01 R:4 X:hhat01 R:4 X:hhat01 R:4
X:bass01 R:4 X:snare01 R:4 X:hhat01 R:4 X:hhat01 R:4
"""

# --- melody.mml content (for testing) ---
melody_mml = """
TEMPO:120
OCTAVE:4
LENGTH:8
VOLUME:75
; Intro - 4 bars (8 quarter notes of rest)
R:1 R:1 R:1 R:1
; Main Groove (Verse 1 & 2) - 16 bars (32 quarter notes)
; C Major
i25:C R:8 i25:D R:8 i25:E R:8 i25:G R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:D R:8
i25:C R:8 i25:D R:8 i25:E R:8 i25:G R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:D R:8
; G Major
i25:G R:8 i25:F R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:D R:8 i25:E R:8 i25:F R:8
i25:G R:8 i25:F R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:D R:8 i25:E R:8 i25:F R:8
; A Minor
i25:A R:8 i25:G R:8 i25:F R:8 i25:E R:8 i25:D R:8 i25:E R:8 i25:F R:8 i25:G R:8
i25:A R:8 i25:G R:8 i25:F R:8 i25:E R:8 i25:D R:8 i25:E R:8 i25:F R:8 i25:G R:8
; F Major
i25:F R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:B3 R:8 i25:C R:8 i25:D R:8 i25:E R:8
i25:F R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:B3 R:8 i25:C R:8 i25:D R:8 i25:E R:8
; Bridge - 8 bars (16 quarter notes of rest)
R:1 R:1 R:1 R:1 R:1 R:1 R:1 R:1
; Main Groove (Verse 3) - 8 bars (16 quarter notes)
; C Major
i25:C R:8 i25:D R:8 i25:E R:8 i25:G R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:D R:8
i25:C R:8 i25:D R:8 i25:E R:8 i25:G R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:D R:8
; G Major
i25:G R:8 i25:F R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:D R:8 i25:E R:8 i25:F R:8
i25:G R:8 i25:F R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:D R:8 i25:E R:8 i25:F R:8
; A Minor
i25:A R:8 i25:G R:8 i25:F R:8 i25:E R:8 i25:D R:8 i25:E R:8 i25:F R:8 i25:G R:8
i25:A R:8 i25:G R:8 i25:F R:8 i25:E R:8 i25:D R:8 i25:E R:8 i25:F R:8 i25:G R:8
; F Major
i25:F R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:B3 R:8 i25:C R:8 i25:D R:8 i25:E R:8
i25:F R:8 i25:E R:8 i25:D R:8 i25:C R:8 i25:B3 R:8 i25:C R:8 i25:D R:8 i25:E R:8
; Outro - 4 bars (8 quarter notes of rest)
R:1 R:1 R:1 R:1
"""

# --- bass.mml content (for testing) ---
bass_mml = """
TEMPO:120
OCTAVE:2
LENGTH:8
VOLUME:80
; Intro - 4 bars (8 quarter notes of rest)
R:1 R:1 R:1 R:1
; Main Groove (Verse 1 & 2) - 16 bars (32 quarter notes)
; I-V-vi-IV progression (C-G-Am-F)
; C Major
sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8
sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8
; G Major
sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8
sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8
; A Minor
sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8
sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8
; F Major
sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8
sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8
; Bridge - 8 bars (16 quarter notes)
; Moving bassline
sqr:C R:8 sqr:D R:8 sqr:E R:8 sqr:F R:8 sqr:G R:8 sqr:A R:8 sqr:B R:8 sqr:C+3 R:8
sqr:C+3 R:8 sqr:B R:8 sqr:A R:8 sqr:G R:8 sqr:F R:8 sqr:E R:8 sqr:D R:8 sqr:C R:8
sqr:C R:8 sqr:D R:8 sqr:E R:8 sqr:F R:8 sqr:G R:8 sqr:A R:8 sqr:B R:8 sqr:C+3 R:8
sqr:C+3 R:8 sqr:B R:8 sqr:A R:8 sqr:G R:8 sqr:F R:8 sqr:E R:8 sqr:D R:8 sqr:C R:8
sqr:C R:8 sqr:D R:8 sqr:E R:8 sqr:F R:8 sqr:G R:8 sqr:A R:8 sqr:B R:8 sqr:C+3 R:8
sqr:C+3 R:8 sqr:B R:8 sqr:A R:8 sqr:G R:8 sqr:F R:8 sqr:E R:8 sqr:D R:8 sqr:C R:8
sqr:C R:8 sqr:D R:8 sqr:E R:8 sqr:F R:8 sqr:G R:8 sqr:A R:8 sqr:B R:8 sqr:C+3 R:8
sqr:C+3 R:8 sqr:B R:8 sqr:A R:8 sqr:G R:8 sqr:F R:8 sqr:E R:8 sqr:D R:8 sqr:C R:8
; Main Groove (Verse 3) - 8 bars (16 quarter notes)
; C Major
sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8
sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8 sqr:C R:8
; G Major
sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8
sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8 sqr:G R:8
; A Minor
sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8
sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8 sqr:A R:8
; F Major
sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8
sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8 sqr:F R:8
; Outro - 4 bars (8 quarter notes of rest)
R:1 R:1 R:1 R:1
"""

# --- harmony.mml content (for testing) ---
harmony_mml = """
TEMPO:120
OCTAVE:4
LENGTH:4
VOLUME:60
; Intro - 4 bars (8 quarter notes of rest)
R:1 R:1 R:1 R:1
; Main Groove (Verse 1 & 2) - 16 bars (32 quarter notes)
; I-V-vi-IV progression (C-G-Am-F)
CHORD:i05:C,i05:E,i05:G ; C Major
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:G,i05:B,i05:D5 ; G Major
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:A,i05:C5,i05:E5 ; A Minor
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:F,i05:A,i05:C5 ; F Major
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
; Bridge - 8 bars (16 quarter notes of rest)
R:1 R:1 R:1 R:1 R:1 R:1 R:1 R:1
; Main Groove (Verse 3) - 8 bars (16 quarter notes)
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:C,i05:E,i05:G
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:G,i05:B,i05:D5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:A,i05:C5,i05:E5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
CHORD:i05:F,i05:A,i05:C5
; Outro - 4 bars (8 quarter notes of rest)
R:1 R:1 R:1 R:1
"""

# --- sfx.mml content (for testing) ---
sfx_mml = """
TEMPO:120
LENGTH:4
VOLUME:100
; Intro - 4 bars (8 quarter notes of rest)
R:1 R:1 R:1 R:1
; Main Groove (Verse 1 & 2) - 16 bars (32 quarter notes)
R:1 R:1 R:1 R:1 R:1 R:1 R:1 R:1
R:1 R:1 R:1 R:1 R:1 R:1 R:1 R:1
; Bridge - 8 bars (16 quarter notes)
; "Whoop" on downbeats
miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4
miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4
miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4
miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4
miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4
miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4 miscellaneous:C4-A4-triangle R:4
miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4
miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4 miscellaneous:D01-i05-208 R:4
; Main Groove (Verse 3) - 8 bars (16 quarter notes)
R:1 R:1 R:1 R:1 R:1 R:1 R:1 R:1
; Outro - 4 bars (8 quarter notes of rest)
R:1 R:1 R:1 R:1
"""


#
# FUNCTIONS
#
def remove_comments(text):
    """
    Removes comments from a string. A comment is defined as everything that
    comes after the first semicolon (;) on a given line. All other formatting,
    including leading/trailing whitespace on a line (before the comment) and
    newline characters, is preserved.

    Args:
        text: The input string, which may contain multiple lines and comments.

    Returns:
        The string with all comments removed.

    Raises:
        TypeError: If the input 'text' is not a string.
    """
    if not isinstance(text, str):
        raise TypeError("Input must be a string.")

    # Split the input string into individual lines.
    # .splitlines() handles different newline characters ('\n', '\r\n', '\r')
    # and does not include empty strings for blank lines, or for a final newline.
    lines = text.splitlines()

    processed_lines = []
    for line in lines:
        # Find the index of the first semicolon on the current line.
        comment_start_index = line.find(';')

        if comment_start_index != -1:
            # If a semicolon is found, take only the part of the line
            # that comes before the semicolon. This preserves any spaces
            # or other characters immediately preceding the semicolon.
            processed_lines.append(line[:comment_start_index])
        else:
            # If no semicolon is found on the line, the entire line is not a comment.
            # So, keep the line as is, preserving all its formatting.
            processed_lines.append(line)

    # Join the processed lines back together using newline characters.
    # Since .splitlines() removes the newline characters, we re-add them.
    # This correctly reconstructs the multi-line string.
    return '\n'.join(processed_lines)


def get_value_after_colon_until_next_command(text, colon_index):
    """
    Extracts the value following a colon at 'colon_index'. The extraction
    stops at the beginning of the next MML command (e.g., ' X:', ' R:', ' tri:'),
    which is identified by a space followed by a word and a colon.
    If no such next command pattern is found, the extraction goes to the earliest of:
    a newline character, a comment character (;), or the end of the string.

    Args:
        text: The input MML string or line.
        colon_index: The index of the colon from which to start extracting the value.
                     The returned substring starts from colon_index + 1.

    Returns:
        The extracted value string. Returns an empty string if no value follows
        the colon or if the 'colon_index' is invalid or at the end of the string.

    Raises:
        TypeError: If 'text' is not a string or 'colon_index' is not an integer.
        ValueError: If 'colon_index' is out of bounds or does not point to a colon.
    """
    if not isinstance(text, str):
        raise TypeError("Input 'text' must be a string.")
    if not isinstance(colon_index, int):
        raise TypeError("Input 'colon_index' must be an integer.")

    # Validate colon_index: it must be within string bounds and point to a colon.
    if not (0 <= colon_index < len(text) and text[colon_index] == ':'):
        raise ValueError(
            f"Invalid colon_index {colon_index}. "
            f"It must be within bounds (0 to {len(text)-1}) and point to a ':' character. "
            f"String: '{text}'"
        )

    # The value starts immediately after the colon
    value_start_index = colon_index + 1

    # If the colon is the last character in the string, there's no value to extract.
    if value_start_index >= len(text):
        return ""

    potential_end_indices = []

    # Regex to find the start of the next MML command pattern:
    # \s     - A single whitespace character
    # \w+    - One or more word characters (alphanumeric + underscore), for the command name
    # :      - A literal colon
    next_command_pattern = re.compile(r'\s\w+:')

    # Search for this pattern starting from `value_start_index`.
    match = next_command_pattern.search(text, pos=value_start_index)
    if match:
        potential_end_indices.append(match.start())

    # Find next newline
    next_newline_idx = text.find('\n', value_start_index)
    if next_newline_idx != -1:
        potential_end_indices.append(next_newline_idx)

    # Find next comment start (;)
    next_comment_idx = text.find(';', value_start_index)
    if next_comment_idx != -1:
        potential_end_indices.append(next_comment_idx)

    # Determine the actual end index based on the earliest found separator
    value_end_index = len(text) # Default to end of string
    if potential_end_indices:
        value_end_index = min(potential_end_indices)

    # Extract the substring and strip any leading/trailing whitespace
    return text[value_start_index:value_end_index].strip()


def get_preceding_word(text, index):
    """
    Returns the word that immediately precedes the given index in a string.
    The function looks backward from the index until it encounters a space,
    a newline character, or the beginning of the string.

    Args:
        text: The input string.
        index: The index in the string from which to start looking backward.

    Returns:
        The word immediately preceding the index. Returns an empty string
        if no word is found (e.g., at the beginning of the string or
        immediately after a space/newline).
    """
    if not (0 <= index <= len(text)):
        raise IndexError("Index out of bounds for the given text.")

    # Start looking backward from the character *before* the given index
    # If index is 0, there's no preceding word.
    if index == 0:
        return ""

    end_of_word_search = index

    # Find the start of the word
    start_of_word_search = end_of_word_search - 1
    while start_of_word_search >= 0:
        char = text[start_of_word_search]
        if char == ' ' or char == '\n':
            break # Found a space or newline, so the word starts after this
        start_of_word_search -= 1

    # Adjust start_of_word_search to be the first character of the word
    # If the loop broke because of a space/newline, increment by 1
    # If the loop reached -1 (start of string), then 0 is the start of the word
    word_start = start_of_word_search + 1

    # Extract the word
    word = text[word_start:end_of_word_search]

    return word


def find_colon_indices_loop(s):
    """
    Finds all indices in a string where the character is a colon (:).

    Args:
        s: The input string.

    Returns:
        A list of integer indices where ':' is found.
    """
    indices = []
    for i in range(len(s)):
        if s[i] == ':':
            indices.append(i)
    return indices


def calculate_mml_duration_v4(mml_content):
    """
    Calculates the total duration of an MML song in seconds.
    Uses a robust tokenization with look-ahead for explicit durations.
    """
    total_duration_seconds = 0.0
    current_tempo_bpm = 120
    current_default_length = 4

    note_length_map = {
        1: 4.0,  # A whole note is 4 beats in 4/4
        2: 2.0,  # Half note is 2 beats
        4: 1.0,  # Quarter note is 1 beat
        8: 0.5,  # Eighth note is 0.5 beats
        16: 0.25, # Sixteenth note is 0.25 beats
        32: 0.125 # Thirty-second note is 0.125 beats
    }

    # Regexes for identifying different types of MML tokens/commands
    re_tempo = re.compile(r'^TEMPO:(\d+)$')
    re_length = re.compile(r'^LENGTH:(\d+)$')
    re_volume = re.compile(r'^VOLUME:(\d+)$')
    re_octave = re.compile(r'^OCTAVE:(\d+)$')

    # Regex for an explicit duration token (e.g., "0.25s")
    re_explicit_duration_token = re.compile(r'^(\d+\.?\d*)s$')

    # Regex for a folder:note or R:length or CHORD:note,note... (base musical commands)
    # This must *not* include explicit seconds, as those are handled separately.
    re_musical_command_base = re.compile(
        r'^(?:'
        r'[a-zA-Z0-9]+:[A-G][\+\-]?\d?[+\-]?|'  # FOLDER:NOTE[octave][accidental] (e.g., sqr:C, i25:D4+)
        r'R:\d+|'                             # R:length (e.g., R:4)
        r'CHORD:[A-Z][\+\-]?\d?,[A-Z][\+\-]?\d?(?:,[A-Z][\+\-]?\d?)*' # CHORD:note,note,...
        r')$'
    )

    for line_num, raw_line in enumerate(mml_content.split('\n')):
        line = raw_line.strip()
        if not line or line.startswith(';'):
            continue

        # Check for global commands that occupy a whole line
        tempo_match = re_tempo.match(line)
        if tempo_match:
            current_tempo_bpm = int(tempo_match.group(1))
            # print(f"L{line_num+1}: SET TEMPO: {current_tempo_bpm} BPM")
            continue

        length_match = re_length.match(line)
        if length_match:
            current_default_length = int(length_match.group(1))
            # print(f"L{line_num+1}: SET LENGTH: {current_default_length}")
            continue

        volume_match = re_volume.match(line)
        if volume_match:
            # print(f"L{line_num+1}: SET VOLUME: {volume_match.group(1)}")
            continue

        octave_match = re_octave.match(line)
        if octave_match:
            # print(f"L{line_num+1}: SET OCTAVE: {octave_match.group(1)}")
            continue

        # Process the rest of the line as a sequence of musical commands
        tokens = line.split()

        idx = 0
        while idx < len(tokens):
            current_token = tokens[idx]

            # Check if the current token is a base musical command (note, rest, chord)
            base_cmd_match = re_musical_command_base.match(current_token)

            if base_cmd_match:
                duration_to_add = 0.0

                # Check for explicit duration in the *next* token
                if idx + 1 < len(tokens):
                    next_token = tokens[idx + 1]
                    explicit_dur_match = re_explicit_duration_token.match(next_token)

                    if explicit_dur_match:
                        duration_s = float(explicit_dur_match.group(1))
                        duration_to_add = duration_s
                        # Consume both current and next token
                        idx += 2
                        # print(f"L{line_num+1}: Parsed explicit: '{current_token} {next_token}', added {duration_to_add:.2f}s. Total: {total_duration_seconds + duration_to_add:.2f}s")
                        total_duration_seconds += duration_to_add
                        continue # Move to next iteration of while loop

                # If no explicit duration found in next token, use default length
                if current_default_length not in note_length_map:
                    print(f"L{line_num+1}: Warning: Unknown LENGTH value {current_default_length}. Skipping command: '{current_token}'")
                    idx += 1
                    continue

                beats = note_length_map[current_default_length]
                duration_s_per_beat = 60.0 / current_tempo_bpm
                duration_to_add = beats * duration_s_per_beat
                total_duration_seconds += duration_to_add
                # print(f"L{line_num+1}: Parsed default: '{current_token}', added {duration_to_add:.2f}s. Total: {total_duration_seconds:.2f}s")
                idx += 1 # Consume current token
                continue # Move to next iteration of while loop

            # If we reach here, the token didn't match a global command (already handled)
            # nor a known musical command (with or without explicit duration).
            # This indicates an unrecognized token or syntax error.
            print(f"L{line_num+1}: Error: Unrecognized MML token or syntax error: '{current_token}' (from line: '{raw_line}')")
            idx += 1 # Advance to avoid infinite loop on bad token

    return total_duration_seconds


# --- Test with the provided MMLs again ---
# (Using the same MML content variables from the previous response)
my_txt = remove_comments(rhythm_mml)
my_colons = find_colon_indices_loop(my_txt)
print("Found %d colons in rhythm.mml" % len(my_colons))

my_lens = {}
cur_len = 4

for i in range(1,10):
    _idx = my_colons[i]
    _word = get_preceding_word(my_txt, _idx)
    _after = get_value_after_colon_until_next_command(my_txt, _idx)
    print(f"{i}th command: {_word.lower()}; after: {_after}")

    if _word == "length":
        # Length redefinition
        cur_len = int(_after)
        print(f"Setting length to {cur_len}")
    elif _word in ["tempo", "octave", "volume"]:
        continue
    elif _word == "r":
        # Rest must have length
        _len = int(_after)
        if _len in my_lens:
            my_lens[_len] += 1
        else:
            my_lens[_len] = 1
    else:
        # TODO: look for explicit duration in CHORD or note; else,
        # count += 1 for cur_len. Lastly, sum across the counts in
        # my_lens.
        pass
