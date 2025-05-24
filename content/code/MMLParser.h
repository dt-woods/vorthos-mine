// MMLParser.h

#ifndef MML_PARSER_H
#define MML_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <variant> // For std::variant (C++17)
#include "NoteDecoder.h"

// --- New structs for parsed command information ---

// Enum to identify the type of command (UPDATED)
enum class CommandType
{
    NOTE,
    TEMPO,
    OCTAVE, // <--- ADD THIS
    LENGTH, // <--- ADD THIS
    // Add other types if your MML expands (e.g., REST, VOLUME)
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

// Update ParsedCommand's variant to include new types
struct ParsedCommand
{
    CommandType type;
    std::string originalCommandString;

    // Add ParsedOctave and ParsedLength to the variant
    std::variant<ParsedNote, ParsedTempo, ParsedOctave, ParsedLength> data;

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
    std::vector<ParsedCommand> debugParseMML(const std::string &mmlString) const;

private:
    std::unique_ptr<NoteDecoder> m_noteDecoder;

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