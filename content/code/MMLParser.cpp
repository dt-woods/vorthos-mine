#include "MMLParser.h"
#include <sstream>   // For std::istringstream
#include <algorithm> // For std::remove_if, std::transform
#include <cctype>    // For std::isspace, std::isdigit etc.
#include <iostream>  // For error reporting
#include <string>    // For std::string::npos, substr etc

// Constructor implementation (MODIFIED)
MMLParser::MMLParser(const std::string &waveformLibraryPath, double defaultTempoBPM, int defaultOctave, int defaultLength)
    : m_noteDecoder(std::make_unique<NoteDecoder>(waveformLibraryPath)),
      m_currentTempoBPM(defaultTempoBPM),
      m_currentOctave(defaultOctave), // Initialize new members
      m_currentLength(defaultLength)  // Initialize new members
{
    // Constructor initializes the NoteDecoder and sets the default global settings.
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
    double currentTempo = m_currentTempoBPM;
    int currentOctave = m_currentOctave;
    int currentLength = m_currentLength;

    std::vector<std::string> command_tokens = splitString(mmlString, ' ');
    size_t i = 0; // Use index for iteration

    while (i < command_tokens.size())
    {
        const std::string &current_token = command_tokens[i];

        if (current_token.empty())
        {
            i++;
            continue;
        }

        size_t colon_pos = current_token.find(':');
        if (colon_pos == std::string::npos)
        {
            std::cerr << "Error: Command '" << current_token << "' is missing a colon ':' delimiter. Skipping." << std::endl;
            i++; // Consume the problematic token
            continue;
        }

        std::string command_type_str = current_token.substr(0, colon_pos);
        std::string current_command_full_string = current_token; // This holds the token that starts the command
        // std::string command_args_str = current_token.substr(colon_pos + 1);

        if (command_type_str == "TEMPO")
        {
            double newTempo = parseDouble(current_token.substr(colon_pos + 1)); // Directly use substr
            if (newTempo > 0)
            {
                currentTempo = newTempo;
                std::cout << "Tempo set to: " << currentTempo << " BPM" << std::endl;
            }
            else
            {
                std::cerr << "Warning: Invalid tempo value '" << current_token.substr(colon_pos + 1) << "' in TEMPO command. Keeping current tempo." << std::endl;
            }
            i++;
            continue;
        }
        else if (command_type_str == "OCTAVE")
        {
            int newOctave = parseInt(current_token.substr(colon_pos + 1), -1); // Directly use substr
            if (newOctave >= 0)
            {
                currentOctave = newOctave;
                std::cout << "Default octave set to: " << currentOctave << std::endl;
            }
            else
            {
                std::cerr << "Warning: Invalid octave value '" << current_token.substr(colon_pos + 1) << "' in OCTAVE command. Keeping current default octave." << std::endl;
            }
            i++;
            continue;
        }
        else if (command_type_str == "LENGTH")
        {
            int newLength = parseInt(current_token.substr(colon_pos + 1), -1); // Directly use substr
            if (newLength > 0 && (newLength == 1 || newLength == 2 || newLength == 4 || newLength == 8 || newLength == 16 || newLength == 32 || newLength == 64))
            {
                currentLength = newLength;
                std::cout << "Default length set to: 1/" << currentLength << " note." << std::endl;
            }
            else
            {
                std::cerr << "Warning: Invalid or unsupported length value '" << current_token.substr(colon_pos + 1) << "' in LENGTH command. Keeping current default length." << std::endl;
            }
            i++;
            continue;
        }

        // --- At this point, it must be a Note/Sound Command ---

        // Check if there's a subsequent token that looks like an explicit duration ("Xs")
        size_t next_token_idx = i + 1;
        if (next_token_idx < command_tokens.size() && isExplicitDurationToken(command_tokens[next_token_idx]))
        {
            current_command_full_string += " " + command_tokens[next_token_idx]; // Combine "X:kick01" and "1s"
            i++;                                                                 // Increment 'i' to consume the duration token along with the current_token
        }

        // Now, extract command_args_str from the potentially combined string
        std::string command_args_str = current_command_full_string.substr(colon_pos + 1); // <--- THIS IS THE KEY FIX

        std::string folderAbbr = command_type_str; // folderAbbr is from current_token
        std::string noteName;
        char accidental;
        int length_for_note;
        int octave_for_note;
        double explicitDurationSeconds;

        if (parseNoteCommand(command_args_str, // Pass the correct combined arguments string
                             folderAbbr, noteName, accidental,
                             length_for_note, octave_for_note, explicitDurationSeconds,
                             currentLength, currentOctave))
        {
            std::vector<float> noteAudio = m_noteDecoder->getNoteAudio(
                folderAbbr,
                noteName,
                accidental,
                length_for_note,
                octave_for_note,
                explicitDurationSeconds, // This should now be correctly parsed!
                currentTempo);
            fullAudioOutput.insert(fullAudioOutput.end(), noteAudio.begin(), noteAudio.end());
        }
        else
        {
            std::cerr << "Error: Failed to parse note command: '" << current_command_full_string << "'" << std::endl;
        }
        i++; // Consume the current_token (and potentially the duration token if it was combined)
    }
    return fullAudioOutput;
}

// debugParseMML Implementation (REVISED for explicit durations)
std::vector<ParsedCommand> MMLParser::debugParseMML(const std::string &mmlString) const
{
    std::vector<ParsedCommand> parsedCommands;
    double currentDebugTempo = 120.0;
    int currentDebugOctave = 4;
    int currentDebugLength = 4;

    std::vector<std::string> command_tokens = splitString(mmlString, ' ');
    size_t i = 0; // Use index for iteration

    while (i < command_tokens.size())
    {
        const std::string &current_token = command_tokens[i];

        if (current_token.empty())
        {
            i++;
            continue;
        }

        ParsedCommand pCmd;
        // The originalCommandString should capture the *entire logical command*,
        // including its explicit duration if it has one. We'll set this below.
        // pCmd.originalCommandString = current_token;

        size_t colon_pos = current_token.find(':');
        if (colon_pos == std::string::npos)
        {
            std::cerr << "Error: Debug: Command '" << current_token << "' is missing a colon ':' delimiter. Marking as UNKNOWN." << std::endl;
            pCmd.type = CommandType::UNKNOWN;
            pCmd.originalCommandString = current_token; // Set for debug output
            parsedCommands.push_back(pCmd);
            i++;
            continue;
        }

        std::string command_type_str = current_token.substr(0, colon_pos);
        std::string command_args_str_initial = current_token.substr(colon_pos + 1); // Store args of initial token

        std::string final_command_full_string = current_token; // Start with the first token of the command

        if (command_type_str == "TEMPO")
        {
            double newTempo = parseDouble(command_args_str_initial);
            if (newTempo > 0)
            {
                currentDebugTempo = newTempo;
                pCmd.type = CommandType::TEMPO;
                pCmd.data = ParsedTempo{newTempo};
            }
            else
            {
                std::cerr << "Warning: Debug: Invalid tempo value '" << command_args_str_initial << "' in TEMPO command. Marking as UNKNOWN." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.originalCommandString = final_command_full_string; // Original string for TEMPO is just current_token
            parsedCommands.push_back(pCmd);
            i++;
            continue;
        }
        else if (command_type_str == "OCTAVE")
        {
            int newOctave = parseInt(command_args_str_initial, -1);
            if (newOctave >= 0)
            {
                currentDebugOctave = newOctave;
                pCmd.type = CommandType::OCTAVE;
                pCmd.data = ParsedOctave{newOctave};
            }
            else
            {
                std::cerr << "Warning: Debug: Invalid octave value '" << command_args_str_initial << "' in OCTAVE command. Marking as UNKNOWN." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.originalCommandString = final_command_full_string; // Original string for OCTAVE is just current_token
            parsedCommands.push_back(pCmd);
            i++;
            continue;
        }
        else if (command_type_str == "LENGTH")
        {
            int newLength = parseInt(command_args_str_initial, -1);
            if (newLength > 0 && (newLength == 1 || newLength == 2 || newLength == 4 || newLength == 8 || newLength == 16 || newLength == 32 || newLength == 64))
            {
                currentDebugLength = newLength;
                pCmd.type = CommandType::LENGTH;
                pCmd.data = ParsedLength{newLength};
            }
            else
            {
                std::cerr << "Warning: Debug: Invalid or unsupported length value '" << command_args_str_initial << "' in LENGTH command. Marking as UNKNOWN." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.originalCommandString = final_command_full_string; // Original string for LENGTH is just current_token
            parsedCommands.push_back(pCmd);
            i++;
            continue;
        }

        // --- At this point, it must be a Note/Sound Command ---

        // Check if there's a subsequent token that looks like an explicit duration ("Xs")
        size_t next_token_idx = i + 1;
        if (next_token_idx < command_tokens.size() && isExplicitDurationToken(command_tokens[next_token_idx]))
        {
            final_command_full_string += " " + command_tokens[next_token_idx]; // Combine "X:kick01" and "1s"
            i++;                                                               // Increment 'i' to consume the duration token along with the current_token
        }

        // Now, extract command_args_str from the potentially combined string
        std::string command_args_for_note_parsing = final_command_full_string.substr(colon_pos + 1); // <--- THIS IS THE KEY FIX

        std::string folderAbbr = command_type_str;
        std::string noteName;
        char accidental;
        int length_for_note;
        int octave_for_note;
        double explicitDurationSeconds;

        if (parseNoteCommand(command_args_for_note_parsing, // Pass the correct combined arguments string
                             folderAbbr, noteName, accidental,
                             length_for_note, octave_for_note, explicitDurationSeconds,
                             currentDebugLength, currentDebugOctave))
        {
            pCmd.type = CommandType::NOTE;
            pCmd.data = ParsedNote{
                folderAbbr,
                noteName,
                accidental,
                length_for_note,
                octave_for_note,
                explicitDurationSeconds};
        }
        else
        {
            std::cerr << "Error: Debug: Failed to parse note command: '" << final_command_full_string << "'." << std::endl;
            pCmd.type = CommandType::UNKNOWN;
        }
        pCmd.originalCommandString = final_command_full_string; // Set original string for note command
        parsedCommands.push_back(pCmd);
        i++;
    }
    return parsedCommands;
}
