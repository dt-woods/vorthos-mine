#include "AudioUtils.h"
#include "MMLParser.h"
#include <sstream>   // For std::istringstream
#include <algorithm> // For std::remove_if, std::transform
#include <cctype>    // For std::isspace, std::isdigit etc.
#include <iostream>  // For error reporting
#include <string>    // For std::string::npos, substr etc

// MMLParser constructor implementation
MMLParser::MMLParser(const std::string &waveformLibraryPath,
                     double defaultTempoBPM,
                     int defaultOctave,
                     int defaultLength,
                     int defaultVolume)
    // Initialize member variables in the initializer list
    : m_noteDecoder(waveformLibraryPath), // Initialize the NoteDecoder here
      m_currentTempoBPM(defaultTempoBPM),
      m_currentOctave(defaultOctave),
      m_currentLength(defaultLength),
      m_currentVolume(static_cast<float>(defaultVolume) / 100.0f)
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
    int defaultLength,
    int defaultOctave
) const
{
    // Initialize output parameters with passed-in defaults
    noteName = "";
    accidental = ' ';
    // Use default length and octave if not found in command_args_str
    length = defaultLength;
    octave = defaultOctave;
    explicitDurationSeconds = 0.0;

    std::vector<std::string> parts = splitString(command_args_str, ' ');

    // --- DEBUG PRINT ---
    std::cout << "  DEBUG: parseNoteCommand received '" << command_args_str << "'. Split into parts:";
    for (const auto &p : parts)
    {
        std::cout << " ['" << p << "']";
    }
    std::cout << std::endl;
    // --- END DEBUG PRINT ---

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

    // Explicit duration parsing
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

    // Special handling for non-pitched instruments
    if (folderAbbr == "X" || folderAbbr == "noise" || folderAbbr == "miscellaneous" || folderAbbr == "sk-5")
    {
        noteName = note_spec_part;
        // length and octave will remain defaultLength/defaultOctave but are unused by NoteDecoder for these types.
        return true;
    }

    // Complex parsing for pitched instruments
    // (MODIFIED to potentially override defaults)
    std::string current_note_spec_remaining = note_spec_part;
    size_t current_pos = 0;

    // 1. Extract baseNote (A-G)
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
            // Use -1 as sentinel for invalid
            int parsedOctave = parseInt(octave_str, -1);
            if (parsedOctave >= 0)
            {
                // Assuming valid octaves are non-negative
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

    // 5. Final check: ensure no unparsed characters remain in
    //    the note_spec_part
    if (current_pos < current_note_spec_remaining.length() && !std::isspace(current_note_spec_remaining[current_pos]))
    {
        std::cerr << "Warning: Unrecognized characters '" << current_note_spec_remaining.substr(current_pos)
                  << "' at end of note specification in '" << command_args_str << "'" << std::endl;
    }

    return true;
}

// parseNoteString Implementation (MODIFIED)
bool MMLParser::parseNoteString(const std::string &fullNoteString,
                                std::string &folderAbbr,
                                std::string &noteName, // This will be the full drum name if X folder
                                char &accidental,
                                int &length_for_note,
                                int &octave_for_note,
                                double &explicitDurationSeconds,
                                int defaultLength,
                                int defaultOctave)
{
    // Initialize output parameters
    noteName = "";
    accidental = ' '; // Initialize to a default non-accidental char
    length_for_note = defaultLength;
    octave_for_note = defaultOctave;
    explicitDurationSeconds = 0.0;
    folderAbbr = ""; // Initialize folderAbbr

    std::string temp_note_str = fullNoteString; // e.g., "X:bass03" or "sqr:C4 0.5s"

    // 1. Extract folder abbreviation (e.g., "X", "sqr", "tri")
    size_t colon_pos = temp_note_str.find(':');
    if (colon_pos != std::string::npos)
    {
        folderAbbr = temp_note_str.substr(0, colon_pos);
        temp_note_str = temp_note_str.substr(colon_pos + 1); // Remaining part: "bass03" or "C4 0.5s"
    }
    else
    {
        // If no folder prefix, use a default (as per your parseMML logic)
        folderAbbr = "sqr";
    }

    // Convert folderAbbr to lowercase for case-insensitive comparison
    std::transform(folderAbbr.begin(), folderAbbr.end(), folderAbbr.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    // 2. Extract explicit duration if present (e.g., "C4 0.5s" -> "0.5s")
    size_t space_pos = temp_note_str.find(' ');
    if (space_pos != std::string::npos)
    {
        std::string duration_part = temp_note_str.substr(space_pos + 1);
        temp_note_str = temp_note_str.substr(0, space_pos); // Note part before space: "C4" or "bass03"

        if (duration_part.length() > 1 && duration_part.back() == 's')
        {
            explicitDurationSeconds = parseDouble(duration_part.substr(0, duration_part.length() - 1));
            if (explicitDurationSeconds <= 0)
            {
                std::cerr << "Warning: Invalid explicit duration '" << duration_part
                          << "' for note '" << fullNoteString << "'. Ignoring explicit duration." << std::endl;
                explicitDurationSeconds = 0.0;
            }
        }
        else
        {
            std::cerr << "Warning: Unrecognized duration format '" << duration_part
                      << "' for note '" << fullNoteString << "'. Ignoring explicit duration." << std::endl;
        }
    }

    // Trim any remaining whitespace from the main note/sound string
    temp_note_str.erase(0, temp_note_str.find_first_not_of(" \t\n\r\f\v"));
    temp_note_str.erase(temp_note_str.find_last_not_of(" \t\n\r\f\v") + 1);

    if (temp_note_str.empty())
    {
        std::cerr << "Error: Empty note string after processing: '" << fullNoteString << "'" << std::endl;
        return false;
    }

    // --- Special handling for non-pitched instruments (like 'X') ---
    // If the folder abbreviation is one of the non-pitched types,
    // the entire 'temp_note_str' is the sound's name (e.g., "bass03", "snare01").
    if (folderAbbr == "x" || folderAbbr == "noise" || folderAbbr == "miscellaneous" || folderAbbr == "sk-5")
    {
        noteName = temp_note_str; // The entire remaining string is the drum/sound ID
        // For non-pitched instruments, 'accidental', 'length', 'octave' are not relevant
        // and will retain their default values/initializations.
        return true; // Successfully parsed a non-pitched sound
    }

    // --- Original logic for PITCHED notes (A-G, accidentals, octave) ---
    // This section is only reached if it's NOT a non-pitched instrument folder.

    size_t i = 0;
    char base_note_char = temp_note_str[i];
    if (!((base_note_char >= 'A' && base_note_char <= 'G') || (base_note_char >= 'a' && base_note_char <= 'g')))
    {
        // This error should now only trigger for truly invalid pitched note names
        std::cerr << "Error: Invalid base note '" << base_note_char
                  << "' in '" << fullNoteString << "'" << std::endl;
        return false;
    }
    noteName = std::string(1, std::toupper(base_note_char)); // Convert to uppercase for consistency
    i++;

    // Check for accidental (+, -, #, b)
    if (i < temp_note_str.length() && (temp_note_str[i] == '+' || temp_note_str[i] == '-' || temp_note_str[i] == '#' || temp_note_str[i] == 'b'))
    {
        accidental = temp_note_str[i];
        if (accidental == '+')
            accidental = '#'; // Normalize '+' to '#'
        if (accidental == '-')
            accidental = 'b'; // Normalize '-' to 'b'
        i++;
    }

    // NO MORE NOTE-SPECIFIC LENGTH; MUST BE SET USING GLOBAL!

    // Extract octave (does not require 'o' anymore)
    if (i < temp_note_str.length() && std::isdigit(temp_note_str[i]))
    {
        std::string octave_str = "";
        while (i < temp_note_str.length() && std::isdigit(temp_note_str[i]))
        {
            octave_str += temp_note_str[i];
            i++;
        }
        // Use parseInt helper for robust conversion
        int parsed_octave = parseInt(octave_str, -1); // Use -1 as sentinel for invalid
        if (parsed_octave >= 0)                       // Assuming valid octaves are non-negative
        {
            octave_for_note = parsed_octave;
        }
        else
        {
            std::cerr << "Warning: Invalid explicit octave in '" << fullNoteString << "'. Using default octave." << std::endl;
            octave_for_note = defaultOctave; // Fallback to default if invalid
        }
    }
    else
    {
        // Octave not specified, use default
        octave_for_note = defaultOctave;
    }

    // Check for any remaining unrecognized characters
    if (i < temp_note_str.length())
    {
        std::cerr << "Warning: Unrecognized characters '" << temp_note_str.substr(i)
                  << "' at end of note specification in '" << fullNoteString << "'" << std::endl;
    }

    // length_for_note is set by defaultLength at the start and is not parsed within this function
    // for pitched notes, unless explicit length syntax like C4/8 or C4L8 is added here.
    // Based on your MML, length is changed by the global LENGTH command.

    return true; // Successfully parsed a pitched note
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

    // --- Use std::stringstream for robust tokenization ---
    std::stringstream ss(cleanedMMLString);
    std::string current_token;

    // The loop now correctly extracts tokens separated by any whitespace
    // (including newlines)
    while (ss >> current_token)
    { // Loop advances token automatically

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
            // Default to squarewave folder if no colon
            command_type_str = "sqr";
            // This is the problem spot for notes without explicit folders.
            // The current MML format always uses folder:note, so this path may not be hit.
            command_args_str = current_token;
        }

        // Convert command_type_str to lowercase for case-insensitive comparison
        std::transform(command_type_str.begin(), command_type_str.end(), command_type_str.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        // --- LOOK-AHEAD LOGIC FOR OPTIONAL EXPLICIT DURATION ---
        // Moved here to allow both CHORD and note explicit duration
        std::string next_token_candidate;
        std::streampos current_iss_pos = ss.tellg(); // Save current stream position

        // Try to read the next token from the stream
        if (ss >> next_token_candidate)
        {
            // *** USE THE HELPER METHOD HERE ***
            if (isExplicitDurationToken(next_token_candidate)) // Check if it's a valid explicit duration token
            {
                // If it's a valid duration, append it to the full note specification.
                // The 'ss >> next_token_candidate' already consumed it, so no rewind needed.
                command_args_str += " " + next_token_candidate;
            }
            else
            {
                // Not an explicit duration, put the token back into the stream
                ss.seekg(current_iss_pos); // Rewind the stream
            }
        }
        // If ss >> next_token_candidate failed (e.g., end of stream),
        // full_note_spec remains just the first token, which is correct.

        // --- END OF LOOK-AHEAD LOGIC ---

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
        else if (command_type_str == "chord")
        { // <--- CHORD Command handling
            std::cout << "DEBUG: Parsing CHORD: " << command_args_str << std::endl;

            double chord_explicitDurationSeconds = 0.0;    // Default to 0.0, indicating no explicit duration
            std::string notes_only_str = command_args_str; // Will hold just the comma-separated notes

            // --- Step 1: Attempt to extract explicit duration from the end of the chord argument string ---
            size_t s_pos = notes_only_str.find_last_of('s');
            // Check if 's' is found and there's a number before it
            if (s_pos != std::string::npos && s_pos > 0 && notes_only_str[s_pos - 1] != ' ')
            {
                // Find the last space before 's', which should separate the notes from the duration
                size_t space_before_s_or_start = notes_only_str.rfind(' ', s_pos);

                // If a space is found, the duration string starts after it. Otherwise, duration starts at the beginning.
                size_t duration_start_pos = (space_before_s_or_start == std::string::npos) ? 0 : space_before_s_or_start + 1;

                if (duration_start_pos < s_pos) // Ensure there's content for a number
                {
                    std::string duration_str = notes_only_str.substr(duration_start_pos, s_pos - duration_start_pos);
                    try
                    {
                        chord_explicitDurationSeconds = std::stod(duration_str);
                        // Successfully parsed duration, now remove it from the string
                        notes_only_str = notes_only_str.substr(0, duration_start_pos);
                        // Trim any trailing whitespace left by removing the duration
                        notes_only_str.erase(notes_only_str.find_last_not_of(" \t\n\r\f\v") + 1);
                        std::cout << "DEBUG: Chord explicit duration found: " << chord_explicitDurationSeconds << "s" << std::endl;
                    }
                    catch (const std::invalid_argument &e)
                    {
                        std::cerr << "Warning: Invalid explicit duration format for chord '" << duration_str << "'. Ignoring." << std::endl;
                        chord_explicitDurationSeconds = 0.0; // Reset to default if parsing failed
                    }
                    catch (const std::out_of_range &e)
                    {
                        std::cerr << "Warning: Explicit duration for chord '" << duration_str << "' out of range. Ignoring." << std::endl;
                        chord_explicitDurationSeconds = 0.0; // Reset to default if parsing failed
                    }
                }
            }

            // --- Step 2: Split the remaining string (which now only contains notes) by commas ---
            std::vector<std::string> note_strings = splitString(notes_only_str, ',');

            std::vector<std::vector<float>> individualNoteAudios;
            size_t chordDurationSamples = 0; // Will now be determined consistently

            if (note_strings.empty())
            {
                std::cerr << "Warning: CHORD command has no notes specified after parsing duration: '" << current_token << "'. Skipping." << std::endl;
                continue;
            }

            // --- Step 3: Parse each note and generate audio, applying the chord-level duration ---
            for (const std::string &note_str_raw : note_strings)
            {
                // Trim whitespace from individual note string parts
                std::string note_str = note_str_raw;
                note_str.erase(0, note_str.find_first_not_of(" \t\n\r\f\v"));
                note_str.erase(note_str.find_last_not_of(" \t\n\r\f\v") + 1);

                if (note_str.empty())
                    continue; // Skip empty strings resulting from multiple commas or leading/trailing commas

                std::string chord_note_folderAbbr;
                std::string chord_note_name;
                char chord_note_accidental;
                int chord_note_length;
                int chord_note_octave;
                double dummy_explicitDurationSeconds; // This will capture any explicit duration within an individual note,
                                                      // but it will be *ignored* in favor of chord_explicitDurationSeconds.

                std::cout << " DEBUG: parseNoteString chord received '" << note_str << "'" << std::endl;

                if (this->parseNoteString(note_str, chord_note_folderAbbr, chord_note_name, chord_note_accidental,
                                          chord_note_length, chord_note_octave, dummy_explicitDurationSeconds,
                                          currentLength, currentOctave))
                {
                    std::vector<float> noteAudio = m_noteDecoder.getNoteAudio(
                        chord_note_folderAbbr,
                        chord_note_name,
                        chord_note_accidental,
                        chord_note_length,
                        chord_note_octave,
                        chord_explicitDurationSeconds, // <--- CRITICAL CHANGE: Use the CHORD-level explicit duration here!
                        currentTempo);

                    // Apply current global volume to individual note
                    for (float &sample : noteAudio)
                    {
                        sample *= currentVolume;
                    }

                    // All notes in the chord will now have the same duration (either explicit or derived from length)
                    // We only need to set the chordDurationSamples once from the first note's size.
                    if (chordDurationSamples == 0)
                    {
                        chordDurationSamples = noteAudio.size();
                    }

                    individualNoteAudios.push_back(noteAudio);
                }
                else
                {
                    std::cerr << "Warning: Could not parse note '" << note_str << "' within CHORD. Skipping this note." << std::endl;
                }
            } // End of loop through note_strings

            std::cout << "DEBUG: After parsing all chord notes - individualNoteAudios count: " << individualNoteAudios.size()
                      << ", calculated chordDurationSamples: " << chordDurationSamples << std::endl;

            // --- Mixing Audio Samples ---
            if (chordDurationSamples > 0 && !individualNoteAudios.empty())
            {
                std::vector<float> mixedChordAudio(chordDurationSamples, 0.0f);
                float maxAmplitude = 0.0f; // To track for clipping

                for (const auto &noteAudio : individualNoteAudios)
                {
                    // Ensure noteAudio has enough samples for chordDurationSamples (should be consistent now)
                    size_t currentNoteAudioSize = noteAudio.size();
                    for (size_t i = 0; i < std::min(chordDurationSamples, currentNoteAudioSize); ++i)
                    {
                        mixedChordAudio[i] += noteAudio[i];
                        // Update max amplitude for clipping detection
                        if (std::abs(mixedChordAudio[i]) > maxAmplitude)
                        {
                            maxAmplitude = std::abs(mixedChordAudio[i]);
                        }
                    }
                }

                // --- Simple Clipping (or add dynamic normalization here if preferred) ---
                if (maxAmplitude > 1.0f)
                {
                    // Option 1: Hard clipping (simpler)
                    for (float &sample : mixedChordAudio)
                    {
                        sample = std::max(-1.0f, std::min(1.0f, sample));
                    }
                    // Option 2: Dynamic normalization (better quality) - uncomment if preferred
                    // float scaleFactor = 1.0f / maxAmplitude;
                    // for (float& sample : mixedChordAudio) {
                    //     sample *= scaleFactor;
                    // }
                    std::cout << "DEBUG: Chord audio clipped/normalized due to high amplitude." << std::endl;
                }

                fullAudioOutput.insert(fullAudioOutput.end(), mixedChordAudio.begin(), mixedChordAudio.end());
            }
            else
            {
                std::cerr << "Warning: No valid notes in CHORD or duration 0. Skipping chord." << std::endl;
            }
        } // End of CHORD block
        else
        { // This is a potential Note/Sound Command (e.g., "X:bass03", "tri:C4")
            // Now, parseNoteString receives the complete note specification,
            // including the explicit duration if one was found and appended.
            std::string full_note_str = command_type_str + ":" + command_args_str;

            std::string parsed_folderAbbr;
            std::string parsed_noteName;
            char parsed_accidental;
            int parsed_length_for_note;
            int parsed_octave_for_note;
            double parsed_explicitDurationSeconds;

            std::cout << " DEBUG: parseNoteString note received '" << full_note_str << "'" << std::endl;

            if (this->parseNoteString(full_note_str, parsed_folderAbbr, parsed_noteName, parsed_accidental,
                                      parsed_length_for_note, parsed_octave_for_note, parsed_explicitDurationSeconds,
                                      currentLength, currentOctave)) // HOTFIX
            {
                std::vector<float> noteAudio = m_noteDecoder.getNoteAudio(
                    parsed_folderAbbr,
                    parsed_noteName,
                    parsed_accidental,
                    parsed_length_for_note,
                    parsed_octave_for_note,
                    parsed_explicitDurationSeconds,
                    currentTempo); // HOTFIX

                // Apply volume
                for (float &sample : noteAudio)
                {
                    sample *= m_currentVolume;
                }
                fullAudioOutput.insert(fullAudioOutput.end(), noteAudio.begin(), noteAudio.end());
            }
            else
            {
                std::cerr << "Error: Could not parse note command '" << command_args_str << "'. Skipping." << std::endl;
            }
        }
    }
    return fullAudioOutput;
}


// --- stripComments function implementation ---
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

        // --- LOOK-AHEAD LOGIC FOR OPTIONAL EXPLICIT DURATION ---
        std::string next_token_candidate;
        std::streampos current_iss_pos = ss.tellg(); // Save current stream position

        // Try to read the next token from the stream
        if (ss >> next_token_candidate)
        {
            // *** USE THE HELPER METHOD HERE ***
            if (isExplicitDurationToken(next_token_candidate)) // Check if it's a valid explicit duration token
            {
                // If it's a valid duration, append it to the full note specification.
                // The 'ss >> next_token_candidate' already consumed it, so no rewind needed.
                command_args_str += " " + next_token_candidate;
            }
            else
            {
                // Not an explicit duration, put the token back into the stream
                ss.seekg(current_iss_pos); // Rewind the stream
            }
        }
        // If ss >> next_token_candidate failed (e.g., end of stream),
        // full_note_spec remains just the first token, which is correct.

        // --- END OF LOOK-AHEAD LOGIC ---

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
        { // <--- VOLUME Debug Handling
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
        else if (command_type_str == "chord")
        {
            ParsedChord parsedChordData;
            std::vector<std::string> note_strings = splitString(command_args_str, ',');

            for (const std::string &note_str : note_strings)
            {
                std::string chord_note_folderAbbr;
                std::string chord_note_name;
                char chord_note_accidental;
                int chord_note_length;
                int chord_note_octave;
                double chord_note_explicitDurationSeconds;

                if (this->parseNoteString(note_str, chord_note_folderAbbr, chord_note_name, chord_note_accidental,
                                          chord_note_length, chord_note_octave, chord_note_explicitDurationSeconds,
                                          currentLength, currentOctave))
                {
                    parsedChordData.notes.push_back(ParsedNote{chord_note_folderAbbr, chord_note_name, chord_note_accidental,
                                                               chord_note_length, chord_note_octave, chord_note_explicitDurationSeconds});
                }
                else
                {
                    std::cerr << "Warning: Debug: Could not parse note '" << note_str << "' within CHORD for debug output. Skipping." << std::endl;
                }
            }
            pCmd.data = parsedChordData;
            pCmd.type = CommandType::CHORD;
        }
        else
        { // Note/Sound Command
            std::string full_note_str = command_type_str + ":" + command_args_str;

            pCmd.type = CommandType::NOTE;
            std::string parsed_folderAbbr;
            std::string parsed_noteName;
            char parsed_accidental;
            int parsed_length_for_note;
            int parsed_octave_for_note;
            double parsed_explicitDurationSeconds;

            if (!this->parseNoteString(full_note_str, parsed_folderAbbr, parsed_noteName, parsed_accidental,
                                       parsed_length_for_note, parsed_octave_for_note, parsed_explicitDurationSeconds,
                                       currentLength, currentOctave))
            {
                std::cerr << "Warning: Debug: Could not parse note command '" << current_token << "'. Marking as UNKNOWN." << std::endl;
                pCmd.type = CommandType::UNKNOWN;
            }
            pCmd.data = ParsedNote{parsed_folderAbbr, parsed_noteName, parsed_accidental,
                                   parsed_length_for_note, parsed_octave_for_note, parsed_explicitDurationSeconds};
        }

        parsedCommands.push_back(pCmd);
    }

    return parsedCommands;
}
