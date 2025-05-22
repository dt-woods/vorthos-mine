#include "NoteDecoder.h" // Include your NoteDecoder class
#include <iostream>
#include <fstream>
#include <vector>

// HOW TO COMPILE:
// g++ mc.cpp NoteDecoder.cpp -o mml_test -lsndfile -std=c++17

// Helper to write raw PCM data (same as before)
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
    // Example: "/home/user/my_waveforms" or "C:/Users/User/Documents/Waveforms"
    const std::string waveformLibraryPath = "/home/user/Dropbox/Music/waveform_library";

    // Output PCM file name
    const std::string outputPcmFile = "test_output.pcm";

    // Overall audio data that will be written to PCM
    std::vector<float> final_audio_output;

    try
    {
        NoteDecoder decoder(waveformLibraryPath);

        // --- Test 1: A4 square wave, 2 seconds, explicit duration ---
        std::cout << "\n--- Testing A4 Square Wave (2s explicit) ---" << std::endl;
        std::vector<float> sqr_a4_audio = decoder.getNoteAudio(
            "sqr", // folderAbbr
            "A",   // noteName
            ' ',   // accidental (natural)
            4,     // length (quarter note - though explicit duration overrides)
            4,     // octave (4)
            2.0,   // explicitDurationSeconds (2 seconds)
            120.0  // currentTempoBPM
        );

        if (!sqr_a4_audio.empty())
        {
            std::cout << "Generated A4 square wave audio size: " << sqr_a4_audio.size() << " samples." << std::endl;
            final_audio_output.insert(final_audio_output.end(), sqr_a4_audio.begin(), sqr_a4_audio.end());
        }
        else
        {
            std::cerr << "Failed to generate A4 square wave audio." << std::endl;
        }

        // --- Test 2: Snare drum, natural duration (no explicit length/duration) ---
        // Add some silence between notes for clarity
        int commonSampleRate = decoder.getLoadedSampleRate();
        size_t silence_samples_1s = commonSampleRate; // For 1 s of silence at this rate
        if (silence_samples_1s > 0)
        {
            final_audio_output.insert(final_audio_output.end(), silence_samples_1s, 0.0f);
            std::cout << "\n--- Adding 1 second of silence ---" << std::endl;
        }

        std::cout << "\n--- Testing Snare Drum (natural duration) ---" << std::endl;
        std::vector<float> snare_audio = decoder.getNoteAudio(
            "X",       // folderAbbr (for casio-drums)
            "snare01", // noteName (style + variant)
            ' ',       // accidental (ignored for drums)
            0,         // length (0, so it uses natural duration)
            0,         // octave (ignored for drums)
            0.0,       // explicitDurationSeconds (0.0, so it uses natural duration)
            120.0      // currentTempoBPM (ignored for natural duration)
        );

        if (!snare_audio.empty())
        {
            std::cout << "Generated Snare audio size: " << snare_audio.size() << " samples." << std::endl;
            final_audio_output.insert(final_audio_output.end(), snare_audio.begin(), snare_audio.end());
        }
        else
        {
            std::cerr << "Failed to generate Snare audio." << std::endl;
        }

        // --- Test 3: Cb5 Triangle wave (should map to B5.wav), 1.5 seconds, derived from length ---
        // Add some silence between notes for clarity
        if (silence_samples_1s > 0)
        {
            final_audio_output.insert(final_audio_output.end(), silence_samples_1s, 0.0f);
            std::cout << "\n--- Adding 1 second of silence ---" << std::endl;
        }

        std::cout << "\n--- Testing Cb5 Triangle Wave (1.5s derived from length) ---" << std::endl;
        // Tempo 60 BPM: quarter note = 1 second. Half note (length 2) = 2 seconds.
        // So, 1.5 seconds is 1.5 * (1/4) = 0.375 of a whole note.
        // This is a bit tricky to map exactly to standard lengths, so we'll use a length
        // that results in roughly 1.5s at 60 BPM, or just note that the calculation
        // will give us 1.5s if currentTempoBPM and length were adjusted.
        // Let's target 1.5 seconds by using length 4 (quarter note) at 40 BPM (60/40 * 4/4 = 1.5s)
        std::vector<float> tri_cb5_audio = decoder.getNoteAudio(
            "tri", // folderAbbr
            "C",   // noteName
            '-',   // accidental (flat)
            4,     // length (quarter note)
            5,     // octave (5)
            0.0,   // explicitDurationSeconds (0.0, so it uses length)
            40.0   // currentTempoBPM (set to 40 for 1.5s quarter note)
        );

        if (!tri_cb5_audio.empty())
        {
            std::cout << "Generated Cb5 triangle wave audio size: " << tri_cb5_audio.size() << " samples." << std::endl;
            final_audio_output.insert(final_audio_output.end(), tri_cb5_audio.begin(), tri_cb5_audio.end());
        }
        else
        {
            std::cerr << "Failed to generate Cb5 triangle wave audio." << std::endl;
        }

        // 4. Write the combined audio to PCM file
        if (!final_audio_output.empty())
        {
            write_pcm_file(outputPcmFile, final_audio_output);
            std::cout << "\nPlayback Info for FFmpeg/VLC (assuming 44100 Hz, 1 channel from your samples):" << std::endl;
            std::cout << "  ffmpeg -f f32le -ar 44100 -ac 1 -i " << outputPcmFile << " output.mp3" << std::endl;
        }
        else
        {
            std::cerr << "No audio generated for output." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "An unhandled error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
