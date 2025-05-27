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
                     int defaultLength,
                     int defaultVolume)
    // Initialize member variables in the initializer list
    : m_noteDecoder(waveformLibraryPath), // Initialize the NoteDecoder member here
      m_currentTempoBPM(defaultTempoBPM),
      m_currentOctave(defaultOctave),
      m_currentLength(defaultLength),
      m_currentVolume(static_cast<float>(defaultVolume) / 100.0f) //
{
    std::cout << "MMLParser initialized with waveform library: " << waveformLibraryPath << std::endl;
    std::cout << "Default Tempo: " << m_currentTempoBPM << ", Octave: " << m_currentOctave << ", Length: " << m_currentLength << ", Volume: " << defaultVolume << "%" << std::endl;
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
    float currentVolume = m_currentVolume;

    std::cout << "Tempo set to: " << currentTempo << " BPM" << std::endl;
    std::cout << "Default octave set to: " << currentOctave << std::endl;
    std::cout << "Default length set to: " << currentLength << std::endl;
    std::cout << "Default volume set to: " << static_cast<int>(currentVolume * 100) << "%" << std::endl;

    // --- Apply comment stripping here ---
    std::string cleanedMMLString = stripComments(mmlString);

    // --- CRITICAL CHANGE HERE: Use std::stringstream for robust tokenization ---
    std::stringstream ss(cleanedMMLString);
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
        else if (command_type_str == "volume")
        { // <--- NEW: VOLUME Command handling
            int volume = parseInt(command_args_str);
            if (volume >= 0 && volume <= 100)
            {
                // Scale to 0.0-1.0
                currentVolume = static_cast<float>(volume) / 100.0f;
                std::cout << "Volume changed to: " << volume << "%" << std::endl;
            }
            else
            {
                std::cerr << "Warning: Invalid volume value '" << command_args_str << "'. Volume must be between 0 and 100. Using current volume." << std::endl;
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
                // --- Apply volume to noteAudio samples ---
                for (float &sample : noteAudio)
                {
                    sample *= currentVolume;
                }
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


// --- NEW: stripComments function implementation ---
std::string MMLParser::stripComments(const std::string &mmlStringWithComments)
{
    std::string cleanedMML;
    std::stringstream ss_input(mmlStringWithComments);
    std::string line;

    while (std::getline(ss_input, line))
    {
        size_t comment_pos = line.find(';'); // Find the first semicolon on the line
        if (comment_pos != std::string::npos)
        {
            // If a semicolon is found, take only the part before it
            cleanedMML += line.substr(0, comment_pos);
        }
        else
        {
            // No semicolon, take the whole line
            cleanedMML += line;
        }
        cleanedMML += " "; // Add a space to separate tokens from different lines
    }
    return cleanedMML;
}


// --- debugParseMML Implementation ---
std::vector<ParsedCommand> MMLParser::debugParseMML(const std::string &mmlFilePath)
{
    std::vector<ParsedCommand> parsedCommands;
    std::string mmlString = readFileIntoString(mmlFilePath);

    if (mmlString.empty())
    {
        std::cerr << "Error: debugParseMML could not read MML file: " << mmlFilePath << std::endl;
        return parsedCommands;
    }

    std::cout << "\n--- Debug Parsing MML String from file: '" << mmlFilePath << "' ---" << std::endl;

    // --- Apply comment stripping here as well ---
    std::string cleanedMMLString = stripComments(mmlString);

    double currentTempo = 120.0;
    int currentOctave = 4;
    int currentLength = 4;
    int currentDebugVolume = 100; // Track volume for debug output

    std::stringstream ss(cleanedMMLString);
    std::string current_token;

    while (ss >> current_token)
    {
        ParsedCommand pCmd;
        pCmd.originalCommandString = current_token;

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
            command_type_str = "sqr";
            command_args_str = current_token;
        }

        std::transform(command_type_str.begin(), command_type_str.end(), command_type_str.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        if (command_type_str == "tempo")
        {
            double tempo = parseDouble(command_args_str);
            if (tempo > 0)
            {
                currentTempo = tempo;
                pCmd.data = ParsedTempo{tempo};
            }
            else
            {
                std::cerr << "Warning: Debug: Invalid tempo value '" << command_args_str << "'. Using current tempo." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.type = CommandType::TEMPO;
        }
        else if (command_type_str == "octave")
        {
            int octave = parseInt(command_args_str);
            if (octave >= 0 && octave <= 8)
            {
                currentOctave = octave;
                pCmd.data = ParsedOctave{octave};
            }
            else
            {
                std::cerr << "Warning: Debug: Invalid octave value '" << command_args_str << "'. Using current octave." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.type = CommandType::OCTAVE;
        }
        else if (command_type_str == "length")
        {
            int length = parseInt(command_args_str);
            if (length > 0 && (length == 1 || length == 2 || length == 4 || length == 8 || length == 16 || length == 32 || length == 64))
            {
                currentLength = length;
                pCmd.data = ParsedLength{length};
            }
            else
            {
                std::cerr << "Warning: Debug: Invalid or unsupported length value '" << command_args_str << "' in LENGTH command. Keeping current default length." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.type = CommandType::LENGTH;
        }
        else if (command_type_str == "volume")
        { // <--- NEW: VOLUME Debug Handling
            int volume = parseInt(command_args_str);
            if (volume >= 0 && volume <= 100)
            {
                currentDebugVolume = volume; // Update debug tracker
                pCmd.data = ParsedVolume{volume};
            }
            else
            {
                std::cerr << "Warning: Debug: Invalid volume value '" << command_args_str << "'. Volume must be between 0 and 100. Marking as UNKNOWN." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.type = CommandType::VOLUME;
        }
        else if (command_type_str == "r")
        {
            // ... (rest handling remains the same) ...
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
                    pCmd.type = CommandType::UNKNOWN;
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
                    pCmd.type = CommandType::UNKNOWN;
                }
            }
            pCmd.data = parsedRestData;
            pCmd.type = CommandType::REST;
        }
        else
        { // Note/Sound Command
            pCmd.type = CommandType::NOTE;
            std::string folderAbbr = command_type_str;
            std::string noteName;
            char accidental;
            int length_for_note = 0;
            int octave_for_note = 0;
            double explicitDurationSeconds = 0.0;

            if (!this->parseNoteCommand(command_args_str, folderAbbr, noteName, accidental,
                                        length_for_note, octave_for_note, explicitDurationSeconds,
                                        currentLength, currentOctave))
            {
                std::cerr << "Warning: Debug: Could not parse note command '" << current_token << "'. Marking as UNKNOWN." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.data = ParsedNote{folderAbbr, noteName, accidental,
                                   length_for_note, octave_for_note, explicitDurationSeconds};
        }

        parsedCommands.push_back(pCmd);
    }

    std::cout << "--- Debug Parsing Complete ---" << std::endl;
    return parsedCommands;
}
