#include "AudioUtils.h"
#include "MMLParser.h"
#include <sstream>   // For std::istringstream
#include <algorithm> // For std::remove_if, std::transform
#include <cctype>    // For std::isspace, std::isdigit etc.
#include <iostream>  // For error reporting
#include <string>    // For std::string::npos, substr etc

// Constructor implementation (MODIFIED)
// MMLParser constructor implementation (CORRECTED)
MMLParser::MMLParser(const std::string &waveformLibraryPath,
                     double defaultTempoBPM,
                     int defaultOctave,
                     int defaultLength)
    // Initialize member variables in the initializer list
    : m_noteDecoder(waveformLibraryPath), // Initialize the NoteDecoder member here
      m_currentTempoBPM(defaultTempoBPM),
      m_currentOctave(defaultOctave),
      m_currentLength(defaultLength)
{
    // Any additional setup if needed
    std::cout << "MMLParser initialized with waveform library: " << waveformLibraryPath << std::endl;
    std::cout << "Default Tempo: " << m_currentTempoBPM << ", Octave: " << m_currentOctave << ", Length: " << m_currentLength << std::endl;
}

// Helper: splitString (Basic implementation)
std::vector<std::string> MMLParser::splitString(const std::string &s, char delimiter) const
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        // Trim leading/trailing whitespace from each token
        token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1);
        token.erase(0, token.find_first_not_of(" \t\n\r\f\v"));
        if (!token.empty())
        {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// Helper: parseInt
int MMLParser::parseInt(const std::string &s, int defaultValue) const
{
    try
    {
        size_t pos;
        int value = std::stoi(s, &pos);
        if (pos == s.length())
        { // Ensure entire string was consumed
            return value;
        }
    }
    catch (const std::exception &)
    {
        // Conversion failed or not a full number
    }
    return defaultValue;
}

// Helper: parseDouble
double MMLParser::parseDouble(const std::string &s, double defaultValue) const
{
    try
    {
        size_t pos;
        double value = std::stod(s, &pos);
        if (pos == s.length())
        { // Ensure entire string was consumed
            return value;
        }
    }
    catch (const std::exception &)
    {
        // Conversion failed or not a full number
    }
    return defaultValue;
}

// parseNoteCommand (MODIFIED to use defaults)
bool MMLParser::parseNoteCommand(
    const std::string &command_args_str,
    std::string &folderAbbr,
    std::string &noteName,
    char &accidental,
    int &length,
    int &octave,
    double &explicitDurationSeconds,
    int defaultLength, // New parameter
    int defaultOctave  // New parameter
) const
{
    // Initialize output parameters with passed-in defaults
    noteName = "";
    accidental = ' ';
    length = defaultLength; // Use default length if not found in command_args_str
    octave = defaultOctave; // Use default octave if not found in command_args_str
    explicitDurationSeconds = 0.0;

    std::vector<std::string> parts = splitString(command_args_str, ' ');

    // --- NEW DEBUG PRINT ---
    std::cout << "  DEBUG: parseNoteCommand received '" << command_args_str << "'. Split into parts:";
    for (const auto &p : parts)
    {
        std::cout << " ['" << p << "']";
    }
    std::cout << std::endl;
    // --- END NEW DEBUG PRINT ---

    std::string note_spec_part;
    if (!parts.empty())
    {
        note_spec_part = parts[0];
    }
    else
    {
        std::cerr << "Error: Empty note specification after colon." << std::endl;
        return false;
    }

    // ... (Existing explicit duration parsing remains the same) ...
    if (parts.size() > 1)
    {
        std::string duration_str = parts[1];
        if (duration_str.length() > 1 && duration_str.back() == 's')
        {
            explicitDurationSeconds = parseDouble(duration_str.substr(0, duration_str.length() - 1));
        }
        else
        {
            std::cerr << "Warning: Malformed explicit duration '" << duration_str << "' in '" << command_args_str << "'" << std::endl;
            explicitDurationSeconds = 0.0;
        }
    }

    // ... (Existing special handling for non-pitched instruments remains the same) ...
    if (folderAbbr == "X" || folderAbbr == "noise" || folderAbbr == "miscellaneous" || folderAbbr == "sk-5")
    {
        noteName = note_spec_part;
        // length and octave will remain defaultLength/defaultOctave but are unused by NoteDecoder for these types.
        return true;
    }

    // --- Complex parsing for pitched instruments (MODIFIED to potentially override defaults) ---
    std::string current_note_spec_remaining = note_spec_part;
    size_t current_pos = 0;

    // 1. Extract baseNote (A-G)
    // ... (This part remains the same) ...
    if (current_pos < current_note_spec_remaining.length() && std::isalpha(current_note_spec_remaining[current_pos]))
    {
        noteName = current_note_spec_remaining.substr(current_pos, 1);
        char upper_note = std::toupper(noteName[0]);
        if (upper_note < 'A' || upper_note > 'G')
        {
            std::cerr << "Error: Invalid base note '" << noteName << "' in '" << command_args_str << "'" << std::endl;
            return false;
        }
        noteName = std::string(1, upper_note);
        current_pos++;
    }
    else
    {
        std::cerr << "Error: Missing base note (A-G) in '" << command_args_str << "'" << std::endl;
        return false;
    }

    // 2. Look for accidental (+ or -)
    // ... (This part remains the same) ...
    if (current_pos < current_note_spec_remaining.length())
    {
        char possible_accidental = current_note_spec_remaining[current_pos];
        if (possible_accidental == '+' || possible_accidental == '-')
        {
            accidental = possible_accidental;
            current_pos++;
        }
    }

    // 3. Parse length (number) - if found, it OVERRIDES the defaultLength
    size_t start_of_length = current_pos;
    while (current_pos < current_note_spec_remaining.length() && std::isdigit(current_note_spec_remaining[current_pos]))
    {
        current_pos++;
    }
    if (current_pos > start_of_length)
    { // If digits were found
        std::string length_str = current_note_spec_remaining.substr(start_of_length, current_pos - start_of_length);
        int parsedLength = parseInt(length_str, 0);
        if (parsedLength > 0)
        {                          // Only override if parsed length is valid
            length = parsedLength; // OVERRIDE DEFAULT LENGTH
        }
        else
        {
            std::cerr << "Warning: Invalid note length '" << length_str << "' in '" << command_args_str << "'. Using default length." << std::endl;
        }
    }
    // If no length digits found, 'length' remains defaultLength.

    // 4. Parse octave ('o' + digit) - if found, it OVERRIDES the defaultOctave
    if (current_pos < current_note_spec_remaining.length() && std::tolower(current_note_spec_remaining[current_pos]) == 'o')
    {
        current_pos++; // Move past 'o'
        if (current_pos < current_note_spec_remaining.length() && std::isdigit(current_note_spec_remaining[current_pos]))
        {
            std::string octave_str = current_note_spec_remaining.substr(current_pos, 1); // Only expecting single digit octave
            int parsedOctave = parseInt(octave_str, -1);                                 // Use -1 as sentinel for invalid
            if (parsedOctave >= 0)
            {                          // Assuming valid octaves are non-negative
                octave = parsedOctave; // OVERRIDE DEFAULT OCTAVE
            }
            else
            {
                std::cerr << "Warning: Octave 'o' found but invalid digit in '" << command_args_str << "'. Using default octave." << std::endl;
            }
            current_pos++;
        }
        else
        {
            std::cerr << "Warning: Octave 'o' found but no digit followed in '" << command_args_str << "'. Using default octave." << std::endl;
            // Octave remains defaultOctave.
        }
    }
    // If 'o' was not found, 'octave' remains defaultOctave.

    // 5. Final check: ensure no unparsed characters remain in the note_spec_part
    // ... (This part remains the same) ...
    if (current_pos < current_note_spec_remaining.length() && !std::isspace(current_note_spec_remaining[current_pos]))
    {
        std::cerr << "Warning: Unrecognized characters '" << current_note_spec_remaining.substr(current_pos)
                  << "' at end of note specification in '" << command_args_str << "'" << std::endl;
    }

    return true;
}

// Helper to check if a string looks like an explicit duration (e.g., "1s", "0.5s")
bool isExplicitDurationToken(const std::string &token)
{
    if (token.empty() || token.back() != 's' || token.length() < 2)
    {
        return false;
    }
    // Check if the part before 's' is a valid number
    std::string num_part = token.substr(0, token.length() - 1);
    bool has_digit = false;
    bool has_decimal = false;
    for (char c : num_part)
    {
        if (std::isdigit(c))
        {
            has_digit = true;
        }
        else if (c == '.')
        {
            if (has_decimal)
                return false; // More than one decimal point
            has_decimal = true;
        }
        else
        {
            return false; // Non-digit, non-decimal character
        }
    }
    return has_digit; // Must have at least one digit
}

// parseMML - Main entry point (REVISED for explicit durations)
std::vector<float> MMLParser::parseMML(const std::string &mmlString)
{
    std::vector<float> fullAudioOutput;

    // Initialize current state (these will be updated by TEMPO, OCTAVE, LENGTH commands)
    double currentTempo = m_currentTempoBPM; // Start with default BPM
    int currentOctave = m_currentOctave;     // Start with default octave
    int currentLength = m_currentLength;     // Start with default length

    std::cout << "Tempo set to: " << currentTempo << " BPM" << std::endl;
    std::cout << "Default octave set to: " << currentOctave << std::endl;
    std::cout << "Default length set to: " << currentLength << std::endl;

    // --- CRITICAL CHANGE HERE: Use std::stringstream for robust tokenization ---
    std::stringstream ss(mmlString);
    std::string current_token;

    // The loop now correctly extracts tokens separated by any whitespace (including newlines)
    while (ss >> current_token)
    { // Loop advances token automatically
        // No more 'i' index or manual 'i++' needed.

        std::string command_type_str;
        std::string command_args_str;

        size_t colon_pos = current_token.find(':');
        if (colon_pos != std::string::npos)
        {
            command_type_str = current_token.substr(0, colon_pos);
            command_args_str = current_token.substr(colon_pos + 1);
        }
        else
        {
            // If no colon, assume it's a note with default folder (e.g., "C4")
            // This logic should match what you have in debugParseMML for notes without explicit folders
            // For example:
            command_type_str = "sqr"; // Default to squarewave folder if no colon
            command_args_str = current_token;
        }

        // Convert command_type_str to lowercase for case-insensitive comparison
        std::transform(command_type_str.begin(), command_type_str.end(), command_type_str.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        if (command_type_str == "tempo")
        {
            double tempo = parseDouble(command_args_str);
            if (tempo > 0)
            {
                currentTempo = tempo;
                std::cout << "Tempo changed to: " << currentTempo << " BPM" << std::endl;
            }
            else
            {
                std::cerr << "Warning: Invalid tempo value '" << command_args_str << "'. Using current tempo." << std::endl;
            }
            // No 'continue;' or 'i++;' needed here, loop handles next token automatically
        }
        else if (command_type_str == "octave")
        {
            int octave = parseInt(command_args_str);
            if (octave >= 0 && octave <= 8)
            {
                currentOctave = octave;
                std::cout << "Octave changed to: " << currentOctave << std::endl;
            }
            else
            {
                std::cerr << "Warning: Invalid octave value '" << command_args_str << "'. Using current octave." << std::endl;
            }
        }
        else if (command_type_str == "length")
        {
            int length = parseInt(command_args_str);
            if (length > 0 && (length == 1 || length == 2 || length == 4 || length == 8 || length == 16 || length == 32 || length == 64))
            {
                currentLength = length;
                std::cout << "Length changed to: " << currentLength << std::endl;
            }
            else
            {
                std::cerr << "Warning: Invalid or unsupported length value '" << command_args_str << "' in LENGTH command. Keeping current default length." << std::endl;
            }
        }
        else if (command_type_str == "r")
        { // REST command
            double restDurationSeconds = 0.0;

            if (command_args_str.length() > 1 && command_args_str.back() == 's')
            {
                restDurationSeconds = parseDouble(command_args_str.substr(0, command_args_str.length() - 1));
            }
            else
            {
                int restLength = parseInt(command_args_str, 0);
                if (restLength > 0 && (restLength == 1 || restLength == 2 || restLength == 4 || restLength == 8 || restLength == 16 || restLength == 32 || restLength == 64))
                {
                    restDurationSeconds = (60.0 / currentTempo) * (4.0 / restLength);
                }
                else
                {
                    std::cerr << "Warning: Invalid or unsupported rest length '" << command_args_str << "' in 'r:' command. Skipping rest." << std::endl;
                    // No audio added, loop proceeds to next token
                }
            }

            if (restDurationSeconds > 0)
            {
                size_t numSamples = static_cast<size_t>(restDurationSeconds * SAMPLE_RATE);
                std::vector<float> silentAudio(numSamples, 0.0f);
                fullAudioOutput.insert(fullAudioOutput.end(), silentAudio.begin(), silentAudio.end());
            }
            else
            {
                std::cerr << "Warning: Rest duration calculated to be 0 or less for '" << current_token << "'. Skipping." << std::endl;
            }
        }
        else
        {                                              // Assume it's a Note/Sound Command (e.g., "sqr:C4", "C4")
            std::string folderAbbr = command_type_str; // folder is the part before colon or default
            std::string noteName;
            char accidental;
            int length_for_note = 0;
            int octave_for_note = 0;
            double explicitDurationSeconds = 0.0;

            std::cout << " DEBUG: parseNoteCommand received '" << command_args_str << "'" << std::endl; // Debug log

            if (this->parseNoteCommand(command_args_str, folderAbbr, noteName, accidental,
                                       length_for_note, octave_for_note, explicitDurationSeconds,
                                       currentLength, currentOctave))
            {
                std::vector<float> noteAudio = m_noteDecoder.getNoteAudio(
                    folderAbbr,
                    noteName,
                    accidental,
                    length_for_note,
                    octave_for_note,
                    explicitDurationSeconds,
                    currentTempo);
                fullAudioOutput.insert(fullAudioOutput.end(), noteAudio.begin(), noteAudio.end());
            }
            else
            {
                std::cerr << "Error: Could not parse note command '" << current_token << "'. Skipping." << std::endl;
            }
        }
    } // End while loop

    return fullAudioOutput;
}

// UPDATED: debugParseMML implementation
std::vector<ParsedCommand> MMLParser::debugParseMML(const std::string &mmlFilePath)
{
    std::vector<ParsedCommand> parsedCommands;
    std::string mmlString = readFileIntoString(mmlFilePath); // <--- Read file content here

    if (mmlString.empty())
    {
        // readFileIntoString already printed an error. Just return empty.
        return parsedCommands;
    }

    std::cout << "\n--- Debug Parsing MML String from file: '" << mmlFilePath << "' ---" << std::endl;

    // Initialize current state (these will be updated by TEMPO, OCTAVE, LENGTH commands)
    double currentTempo = 120.0;
    int currentOctave = 4;
    int currentLength = 4;

    // Tokenize the MML string
    std::stringstream ss(mmlString);
    std::string current_token;

    // This loop now iterates through tokens parsed from the file content
    while (ss >> current_token)
    {
        ParsedCommand pCmd;
        pCmd.originalCommandString = current_token; // Store original for debug output
        CommandType type = CommandType::UNKNOWN;
        std::string command_type_str;
        std::string command_args_str;

        size_t colon_pos = current_token.find(':');
        if (colon_pos != std::string::npos)
        {
            command_type_str = current_token.substr(0, colon_pos);
            command_args_str = current_token.substr(colon_pos + 1);
        }
        else
        {
            // If no colon, assume it's a note with default folder (e.g., "C4")
            // This is a simplification for basic MML, adjust if your grammar allows non-colon notes
            command_type_str = "note"; // A placeholder type to fall into note parsing
            command_args_str = current_token;
        }

        // Convert command_type_str to lowercase for case-insensitive comparison
        std::transform(command_type_str.begin(), command_type_str.end(), command_type_str.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        if (command_type_str == "tempo")
        {
            type = CommandType::TEMPO;
            double tempo = parseDouble(command_args_str);
            if (tempo > 0)
            {
                currentTempo = tempo;
                pCmd.data = ParsedTempo{tempo};
            }
            else
            {
                std::cerr << "Warning: Invalid tempo value '" << command_args_str << "'. Using current tempo." << std::endl;
                type = CommandType::UNKNOWN; // Mark as unknown if value is invalid
            }
        }
        else if (command_type_str == "octave")
        {
            type = CommandType::OCTAVE;
            int octave = parseInt(command_args_str);
            if (octave >= 0 && octave <= 8)
            { // Typical MIDI octave range
                currentOctave = octave;
                pCmd.data = ParsedOctave{octave};
            }
            else
            {
                std::cerr << "Warning: Invalid octave value '" << command_args_str << "'. Using current octave." << std::endl;
                type = CommandType::UNKNOWN;
            }
        }
        else if (command_type_str == "length")
        {
            type = CommandType::LENGTH;
            int length = parseInt(command_args_str);
            // Valid common lengths for musical notes
            if (length > 0 && (length == 1 || length == 2 || length == 4 || length == 8 || length == 16 || length == 32 || length == 64))
            {
                currentLength = length;
                pCmd.data = ParsedLength{length};
            }
            else
            {
                std::cerr << "Warning: Invalid length value '" << command_args_str << "'. Using current length." << std::endl;
                type = CommandType::UNKNOWN;
            }
        }
        else if (command_type_str == "r")
        { // REST command
            type = CommandType::REST;
            ParsedRest parsedRestData;
            parsedRestData.isExplicitDuration = false;
            parsedRestData.length = 0;
            parsedRestData.explicitDurationSeconds = 0.0;

            if (command_args_str.length() > 1 && command_args_str.back() == 's')
            {
                double explicitRestDur = parseDouble(command_args_str.substr(0, command_args_str.length() - 1));
                if (explicitRestDur > 0)
                {
                    parsedRestData.explicitDurationSeconds = explicitRestDur;
                    parsedRestData.isExplicitDuration = true;
                }
                else
                {
                    std::cerr << "Warning: Debug: Invalid or non-positive explicit rest duration '" << command_args_str << "' in 'r:' command. Marking as UNKNOWN." << std::endl;
                    type = CommandType::UNKNOWN;
                }
            }
            else
            {
                int restLength = parseInt(command_args_str, 0);
                if (restLength > 0 && (restLength == 1 || restLength == 2 || restLength == 4 || restLength == 8 || restLength == 16 || restLength == 32 || restLength == 64))
                {
                    parsedRestData.length = restLength;
                }
                else
                {
                    std::cerr << "Warning: Debug: Invalid or unsupported rest length '" << command_args_str << "' in 'r:' command. Marking as UNKNOWN." << std::endl;
                    type = CommandType::UNKNOWN;
                }
            }
            pCmd.data = parsedRestData; // Assign rest data whether valid or not (type is UNKNOWN if invalid)
        }
        else
        { // Assume it's a Note/Sound Command (e.g., "sqr:C4", "tri:D+")
            type = CommandType::NOTE;
            std::string folderAbbr = command_type_str; // folder is the part before colon
            std::string noteName;
            char accidental;
            int length_for_note = 0;
            int octave_for_note = 0;
            double explicitDurationSeconds = 0.0;

            // Call parseNoteCommand - assumes it's a standalone function or member
            // If parseNoteCommand is a private member, you'll need to call it with `this->`
            // If it's a non-member helper, ensure it's accessible.
            // For now, let's assume it's a private member function that handles parsing.
            // This is the function that parses "C4" or "D+ 1s"
            if (!this->parseNoteCommand(command_args_str, folderAbbr, noteName, accidental,
                                        length_for_note, octave_for_note, explicitDurationSeconds,
                                        currentLength, currentOctave))
            {
                std::cerr << "Warning: Could not parse note command '" << current_token << "'. Marking as UNKNOWN." << std::endl;
                type = CommandType::UNKNOWN;
            }
            // Populate ParsedNote
            pCmd.data = ParsedNote{folderAbbr, noteName, accidental,
                                   length_for_note, octave_for_note, explicitDurationSeconds};
        }

        pCmd.type = type;               // Set the determined command type
        parsedCommands.push_back(pCmd); // Add to our list

    } // End while loop

    std::cout << "--- Debug Parsing Complete ---" << std::endl;
    return parsedCommands;
}
