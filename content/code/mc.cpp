// main.cpp

#include "MMLParser.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <variant> // Required for std::visit

// Helper to write raw PCM file (for actual audio testing)
void write_pcm_file(const std::string &filename, const std::vector<float> &audio_data)
{
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile)
    {
        std::cerr << "Error opening output file: " << filename << std::endl;
        return;
    }
    outfile.write(reinterpret_cast<const char *>(audio_data.data()), audio_data.size() * sizeof(float));
    outfile.close();
    std::cout << "Audio data written to " << filename << " (raw float)." << std::endl;
}

int main()
{
    // IMPORTANT: Replace with the actual base path to your waveform library!
    const std::string waveformLibraryPath = "/path/to/your/waveform/library"; // <--- CHANGE THIS!

    try
    {
        // Initialize parser with default settings (e.g., o4, length 4)
        MMLParser parser(waveformLibraryPath, 120.0, 4, 4);

        // --- Test MML Strings (UPDATED FOR NEW SYNTAX) ---
        std::string mml1 = "TEMPO:150 OCTAVE:5 LENGTH:8 sqr:A tri:C+ X:kick01 1s";       // A and C+ should be o5, length 8
        std::string mml2 = "TEMPO:90 imp:G+2o3 4s noise:white1s 1s";                     // G+ should be o3, length 2
        std::string mml3 = "OCTAVE:3 LENGTH:16 sqr:C tri:E+o4 imp:G- 0.5s";              // C should be o3, length 16. E+ should be o4, length 16. G- explicit duration.
        std::string mml4 = "invalid_command sqr:A+4o4o4 TEMPO:abc OCTAVE:xyz LENGTH:1s"; // Test invalid/malformed commands
        std::string mml5 = "sqr:C tri:D+ imp:E-";                                        // Should use initial defaults (o4, length 4)

        std::vector<std::string> testMMLs = {mml1, mml2, mml3, mml4, mml5};

        for (int i = 0; i < testMMLs.size(); ++i)
        {
            std::cout << "\n--- Debug Parsing MML String " << (i + 1) << ": '" << testMMLs[i] << "' ---" << std::endl;
            std::vector<ParsedCommand> commands = parser.debugParseMML(testMMLs[i]);

            for (const auto &cmd : commands)
            {
                std::cout << "  Original: '" << cmd.originalCommandString << "' -> ";

                // Use std::visit to safely access variant data
                std::visit([&](auto &&arg)
                           {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, ParsedNote>) {
                        std::cout << "NOTE { Folder: " << arg.folderAbbr
                                  << ", Name: " << arg.noteName
                                  << ", Accidental: '" << (arg.accidental == ' ' ? "natural" : std::string(1, arg.accidental)) << "'"
                                  << ", Length: " << arg.length
                                  << ", Octave: " << arg.octave
                                  << ", Explicit Duration: " << arg.explicitDurationSeconds << "s }";
                    } else if constexpr (std::is_same_v<T, ParsedTempo>) {
                        std::cout << "TEMPO { Value: " << arg.value << " BPM }";
                    } else if constexpr (std::is_same_v<T, ParsedOctave>) { // New output for Octave
                        std::cout << "OCTAVE { Value: " << arg.value << " }";
                    } else if constexpr (std::is_same_v<T, ParsedLength>) { // New output for Length
                        std::cout << "LENGTH { Value: " << arg.value << " }";
                    } }, cmd.data);

                if (cmd.type == CommandType::UNKNOWN)
                { // Handle UNKNOWN explicitly if variant isn't handled for it
                    std::cout << "UNKNOWN COMMAND";
                }
                std::cout << std::endl;
            }
        }

        // Example of actual audio generation (uncomment and test if you wish)
        // std::cout << "\n--- Generating Audio for mml1 ---" << std::endl;
        // std::vector<float> final_audio = parser.parseMML(mml1);
        // if (!final_audio.empty()) {
        //     write_pcm_file("mml1_output.pcm", final_audio);
        // }
    }
    catch (const std::exception &e)
    {
        std::cerr << "An unhandled error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}