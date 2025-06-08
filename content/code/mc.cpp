// main.cpp
#include "AudioUtils.h"
#include "MMLParser.h"
#include "NoteDecoder.h"
#include <iostream>
#include <fstream> // Required for file operations

// COMPILE:
// g++ main.cpp MMLParser.cpp NoteDecoder.cpp AudioUtils.cpp -o mml_player -lsndfile -std=c++17
// USE:
// ./mml_player /path/to/your/waveform/library song.mml
int main(int argc, char *argv[])
{
    // --- Parse Command Line Arguments ---
    if (argc < 3)
    { // Now expecting at least 3 arguments: program_name, waveform_path, mml_file_path
        std::cerr << "Usage: " << argv[0] << " <waveform_library_path> <mml_file_path> [output_pcm_filename]" << std::endl;
        return 1;
    }

    std::string waveformLibraryPath = argv[1]; // First argument is the waveform library path

    // --- Normalize waveformLibraryPath: remove trailing slash if present ---
    if (!waveformLibraryPath.empty())
    {                                               // Ensure the string is not empty
        char lastChar = waveformLibraryPath.back(); // Get the last character
        if (lastChar == '/' || lastChar == '\\')
        {                                   // Check for both forward and backward slashes
            waveformLibraryPath.pop_back(); // Remove the last character
            std::cout << "Normalized waveformLibraryPath to: " << waveformLibraryPath << std::endl;
        }
    }

    std::string mmlFilePath = argv[2];                  // Second argument is the MML file path
    std::string outputPcmFilename = "output_audio.pcm"; // Default output filename

    if (argc > 3)
    { // If there's a third argument, it's the custom output filename
        outputPcmFilename = argv[3];
    }

    // --- Instantiate Parser ---
    MMLParser parser(waveformLibraryPath, 120.0, 4, 4, 100);

    // --- DEBUG PARSING ---
    std::vector<ParsedCommand> debugOutput = parser.debugParseMML(mmlFilePath);
    for (const auto &cmd : debugOutput)
    {
        std::cout << "Original: '" << cmd.originalCommandString << "' -> ";
        if (cmd.type == CommandType::NOTE)
        {
            const auto &note = std::get<ParsedNote>(cmd.data);
            std::cout << "NOTE { Folder: " << note.folderAbbr
                      << ", Name: " << note.noteName
                      << ", Accidental: '" << note.accidental
                      << "', Length: " << note.length
                      << ", Octave: " << note.octave
                      << ", Explicit Duration: " << note.explicitDurationSeconds << "s }";
        }
        else if (cmd.type == CommandType::TEMPO)
        {
            const auto &tempo = std::get<ParsedTempo>(cmd.data);
            std::cout << "TEMPO { BPM: " << tempo.value << " }";
        }
        else if (cmd.type == CommandType::OCTAVE)
        {
            const auto &octave = std::get<ParsedOctave>(cmd.data);
            std::cout << "OCTAVE { Value: " << octave.value << " }";
        }
        else if (cmd.type == CommandType::LENGTH)
        {
            const auto &length = std::get<ParsedLength>(cmd.data);
            std::cout << "LENGTH { Value: " << length.value << " }";
        }
        else if (cmd.type == CommandType::REST)
        {
            const auto &rest = std::get<ParsedRest>(cmd.data);
            std::cout << "REST { "
                      << (rest.isExplicitDuration ? "Explicit Duration: " + std::to_string(rest.explicitDurationSeconds) + "s" : "Length: " + std::to_string(rest.length))
                      << " }";
        }
        else if (cmd.type == CommandType::VOLUME)
        {
            const auto &volume = std::get<ParsedVolume>(cmd.data);
            std::cout << "VOLUME { Value: " << volume.value << "% }";
        }
        else if (cmd.type == CommandType::CHORD)
        { // <--- NEW: CHORD Debug Output
            const auto &chord = std::get<ParsedChord>(cmd.data);
            std::cout << "CHORD { Notes: [";
            bool firstNote = true;
            for (const auto &note : chord.notes)
            {
                if (!firstNote)
                    std::cout << ", ";
                std::cout << note.folderAbbr << ":" << note.noteName << note.accidental << note.octave; // Simplified for brevity
                firstNote = false;
            }
            std::cout << "] }";
        }
        else
        {
            std::cout << "UNKNOWN Command";
        }
        std::cout << std::endl;
    }

    // Rest of audio generation

    std::cout << "\n--- Generating Audio from " << mmlFilePath << " ---" << std::endl;

    // --- Generate Audio ---
    // Read MML from file *again* for parseMML (or you could modify parseMML to take path too, but often easier to reuse readFileIntoString)
    std::string mmlStringForAudio = readFileIntoString(mmlFilePath);
    if (mmlStringForAudio.empty())
    {
        std::cerr << "Failed to read MML for audio generation." << std::endl;
        return 1;
    }

    std::vector<float> audioOutput = parser.parseMML(mmlStringForAudio); // Pass the string here

    // --- Read MML from file ---
    std::string mmlString = readFileIntoString(mmlFilePath);
    if (mmlString.empty())
    {
        // readFileIntoString already prints an error message
        return 1;
    }

    std::cout << "--- Parsing MML from " << mmlFilePath << " ---" << std::endl;

    if (audioOutput.empty())
    {
        std::cerr << "Parsing generated no audio data." << std::endl;
        return 1;
    }

    // --- Save Audio to PCM File ---
    if (saveToPcmFile(audioOutput, outputPcmFilename))
    {
        std::cout << "Audio saved to " << outputPcmFilename << std::endl;
        std::cout << "To play or convert this raw PCM file, you might use tools like FFmpeg or Audacity:" << std::endl;
        std::cout << "  Using FFmpeg: ffmpeg -f f32le -ar " << SAMPLE_RATE << " -ac 1 -i " << outputPcmFilename << " output_audio.wav" << std::endl;
        std::cout << "  (Note: f32le is 32-bit float, little-endian; -ac 1 assumes mono.)" << std::endl;
        std::cout << "  Using Audacity: File > Import > Raw Data... then specify Sample Rate (" << SAMPLE_RATE << " Hz), Format (32-bit float), Channels (1 Mono)." << std::endl;
    }
    else
    {
        std::cerr << "Failed to save audio to PCM file." << std::endl;
        return 1;
    }

    return 0;
}
