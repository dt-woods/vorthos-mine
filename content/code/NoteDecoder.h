// NoteDecoder.h

#ifndef NOTE_DECODER_H
#define NOTE_DECODER_H

#include <string>
#include <vector>
#include <map>
#include <sndfile.h> // For SF_INFO and related types

// Structure to hold information about a loaded waveform sample
struct SampleInfo
{
    std::string filePath; // Store the file path for cache key
    std::vector<float> data;
    int sampleRate;
    int channels;
    double durationSeconds; // Pre-calculated duration of the original sample

    // Default constructor to ensure members are initialized
    SampleInfo() : sampleRate(0), channels(0), durationSeconds(0.0) {}
};

// Forward declaration of the generate_audio function (from previous discussions)
// This function takes raw sample data, its sample rate, and a desired duration,
// and returns the looped/cut audio data.
std::vector<float> generate_audio(const std::vector<float> &sample_data, double sample_rate, double desired_duration);

class NoteDecoder
{
public:
    // Constructor: Takes the base path to your waveform library
    NoteDecoder(const std::string &libraryBasePath);

    int getLoadedSampleRate() const;

    // Main function to get audio data for a single MML note command
    // Parameters would come from your MML parser
    // For now, let's pass them explicitly for testing
    // The tempo (BPM) is needed to calculate absolute duration from note length
    std::vector<float> getNoteAudio(
        const std::string &folderAbbr,
        const std::string &noteName,    // e.g., "A", "C", "base", "white"
        char accidental,                // '+', '-', or ' ' (space/empty for natural)
        int length,                     // e.g., 4 (quarter), 8 (eighth). 0 if not specified.
        int octave,                     // e.g., 4 (octave 4). 0 if not specified (for drums/noise)
        double explicitDurationSeconds, // The 'Xs' duration from MML. Use 0 if not specified.
        double currentTempoBPM          // The current tempo for calculating duration from 'length'
    );

private:
    std::string m_libraryBasePath;
    // Cache for loaded waveform samples.
    // Key could be the full WAV file path or a standardized ID.
    std::map<std::string, SampleInfo> m_sampleCache;

    // Helper functions:

    // Maps MML folder abbreviations to full directory names
    std::string getFullFolderPath(const std::string &folderAbbr) const;

    // Constructs the full WAV file path from MML parameters
    std::string buildWaveformFilePath(
        const std::string &folderAbbr,
        const std::string &noteName,
        char accidental,
        int length,
        int octave) const;

    // Loads a .wav file and stores it in the cache
    SampleInfo loadWavFile(const std::string &filePath);

    // Translates MML note name, accidental, and octave into a canonical
    // filename part (e.g., "A4", "C#5", "Bb3")
    std::string getCanonicalNoteFilename(
        const std::string &noteName,
        char accidental, int octave) const;

    // Converts MML note length (e.g., 4, 8) and tempo to duration in s
    // This is optional if we always use explicitDurationSeconds from MML,
    // but useful if 'length' should determine duration.
    double calculateDurationFromLength(
        int length,
        double currentTempoBPM) const;
};

#endif // NOTE_DECODER_H