// MMLParser.h

#ifndef MML_PARSER_H
#define MML_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <variant> // For std::variant (C++17)
#include "AudioUtils.h"
#include "NoteDecoder.h"

// Define the global audio sample rate
const int SAMPLE_RATE = 44100; // <--- ADD THIS LINE (Adjust value if needed)

// --- New structs for parsed command information ---

// Enum to identify the type of command (UPDATED)
enum class CommandType
{
    NOTE,
    TEMPO,
    OCTAVE,
    LENGTH,
    REST,
    // Add other types if your MML expands (e.g., VOLUME)
    UNKNOWN
};

// Struct for parsed Note commands
struct ParsedNote
{
    std::string folderAbbr;
    std::string noteName;
    char accidental;
    int length;
    int octave;
    double explicitDurationSeconds;
    // Add any other note-specific properties parsed from MML
};

// Struct for parsed Tempo commands
struct ParsedTempo
{
    double value; // The BPM value
};

// --- New Structs for Parsed Global Commands ---
struct ParsedOctave
{
    int value;
};

struct ParsedLength
{
    int value;
};

struct ParsedRest
{
    int length;                     // e.g., 4 for a quarter rest
    double explicitDurationSeconds; // e.g., 2.5 for 2.5 seconds of rest
    bool isExplicitDuration;        // True if explicitDurationSeconds is used
};

// Update ParsedCommand's variant to include new types
struct ParsedCommand
{
    CommandType type;
    std::string originalCommandString;

    // Add ParsedOctave and ParsedLength to the variant
    std::variant<ParsedNote, ParsedTempo, ParsedOctave, ParsedLength, ParsedRest> data;

    // Removed the helper comment as std::get/std::get_if are standard access methods.
};

// --- End new structs ---

class MMLParser
{
public:
    // Constructor: Takes the base path for waveforms and default global settings
    MMLParser(const std::string &waveformLibraryPath,
              double defaultTempoBPM = 120.0,
              int defaultOctave = 4,  // New default for initial octave
              int defaultLength = 4); // New default for initial length (e.g., quarter note)

    std::vector<float> parseMML(const std::string &mmlString);
    // UPDATED: debugParseMML now takes a file path
    std::vector<ParsedCommand> debugParseMML(const std::string &mmlFilePath);

private:
    NoteDecoder m_noteDecoder; // <--- This is the change

    // Member variables for current global settings
    double m_currentTempoBPM;
    int m_currentOctave; // Tracks the current default octave
    int m_currentLength; // Tracks the current default length

    // Helper functions (declarations)
    std::vector<std::string> splitString(const std::string &s, char delimiter) const;
    int parseInt(const std::string &s, int defaultValue = 0) const;
    double parseDouble(const std::string &s, double defaultValue = 0.0) const;

    // Modified parseNoteCommand to accept current default length and octave
    bool parseNoteCommand(
        const std::string &command_args_str,
        std::string &folderAbbr,
        std::string &noteName,
        char &accidental,
        int &length, // Output: actual length used for this note
        int &octave, // Output: actual octave used for this note
        double &explicitDurationSeconds,
        int defaultLength, // Input: current default length
        int defaultOctave  // Input: current default octave
    ) const;
};

#endif // MML_PARSER_H