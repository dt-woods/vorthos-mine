#include "NoteDecoder.h" // Assuming you create this header
#include <map>
#include <iostream> // For warning/error output during development
#include <string>
#include <stdexcept> // For throwing errors on unsupported MML
#include <sstream>   // For building strings with numbers
#include <algorithm> // For std::tolower (optional, for case-insensitive names)

//////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS                                                        //
//////////////////////////////////////////////////////////////////////////////

// --- generate_audio function (from previous discussions) ---
std::vector<float> generate_audio(const std::vector<float> &sample_data, double sample_rate, double desired_duration)
{
    std::vector<float> output_audio;
    if (sample_rate <= 0 || sample_data.empty() || desired_duration <= 0)
    {
        // Return empty for invalid input
        return output_audio;
    }

    double sample_length_seconds = static_cast<double>(sample_data.size()) / sample_rate;

    if (desired_duration <= sample_length_seconds)
    {
        // If the desired duration is shorter than the sample, take a segment
        size_t num_samples_to_take = static_cast<size_t>(desired_duration * sample_rate);
        if (num_samples_to_take > sample_data.size())
        { // Safety check
            num_samples_to_take = sample_data.size();
        }
        output_audio.insert(output_audio.end(), sample_data.begin(), sample_data.begin() + num_samples_to_take);
    }
    else
    {
        // If the desired duration is longer, loop the sample
        double num_loops_exact = desired_duration / sample_length_seconds;
        size_t num_full_loops = static_cast<size_t>(num_loops_exact);
        double remaining_duration = desired_duration - (num_full_loops * sample_length_seconds);
        size_t num_remaining_samples = static_cast<size_t>(remaining_duration * sample_rate);

        // Add the full loops
        output_audio.reserve(num_full_loops * sample_data.size() + num_remaining_samples); // Pre-allocate
        for (size_t i = 0; i < num_full_loops; ++i)
        {
            output_audio.insert(output_audio.end(), sample_data.begin(), sample_data.end());
        }

        // Add the remaining part of the sample (if any)
        if (num_remaining_samples > 0)
        {
            if (num_remaining_samples > sample_data.size())
            { // Safety check
                num_remaining_samples = sample_data.size();
            }
            output_audio.insert(output_audio.end(), sample_data.begin(), sample_data.begin() + num_remaining_samples);
        }
    }
    return output_audio;
}

//////////////////////////////////////////////////////////////////////////////
// Class Constructor                                                        //
//////////////////////////////////////////////////////////////////////////////

// --- NoteDecoder Constructor (minimal for now) ---
NoteDecoder::NoteDecoder(const std::string &libraryBasePath) : m_libraryBasePath(libraryBasePath)
{
    // Optional: Add some initialization or validation here
    // std::cout << "NoteDecoder initialized with library base path: " << m_libraryBasePath << std::endl;
}


//////////////////////////////////////////////////////////////////////////////
// Class Functions                                                          //
//////////////////////////////////////////////////////////////////////////////

std::string NoteDecoder::buildWaveformFilePath(
    const std::string &folderAbbr,
    const std::string &noteName,
    char accidental,
    int length, // MML note length (not directly used for filename here)
    int octave  // MML octave
) const
{
    std::string fullFolderPath = m_libraryBasePath + "/" + getFullFolderPath(folderAbbr);
    std::string filename;

    // Use a stringstream for easy number to string conversion for variants
    std::stringstream ss;

    if (folderAbbr == "x")   // HOTFIX: all folders are lowercase
    { // Casio Drums
        // Naming: DRUMS-<style><variant>.wav
        std::string drumStyle = noteName;
        std::string drumVariant = "01"; // Default variant

        // Check if noteName contains a variant number (e.g., "base06")
        // Assuming variant is always 2 digits at the end of the style name
        if (drumStyle.length() >= 2 && isdigit(drumStyle[drumStyle.length() - 2]) && isdigit(drumStyle[drumStyle.length() - 1]))
        {
            drumVariant = drumStyle.substr(drumStyle.length() - 2);
            drumStyle = drumStyle.substr(0, drumStyle.length() - 2); // Remove variant from style
        }
        // Note: For drums, accidental, length, octave are usually ignored.
        // They might be used for other effects later, but not for path.

        ss << "DRUMS-" << drumStyle << drumVariant << ".wav";
        filename = ss.str();
    }
    else if (folderAbbr == "noise")
    {
        // Naming: <type>-<duration>s.wav (e.g., white-1s.wav, pink-7s.wav)
        // noteName already includes type and duration, e.g., "white1s", "pink7s"
        // We'll assume the MML input will match the filename exactly for noise.
        // So, if MML is "noise:white1s", we look for "white-1s.wav"
        // If MML is "noise:pink7s", we look for "pink-7s.wav"
        // This means the 'name' part of MML for noise needs to be 'white1s' or 'pink7s' to match the actual file naming.
        // We need to add the dash back in the filename.
        std::string noise_type_and_duration = noteName; // e.g., "white1s"
        size_t last_digit_pos = noise_type_and_duration.find_last_not_of("0123456789") + 1;
        if (last_digit_pos < noise_type_and_duration.length())
        {
            // Insert dash before the duration part if it exists
            noise_type_and_duration.insert(last_digit_pos, "-");
        }
        ss << noise_type_and_duration << ".wav";
        filename = ss.str();
    }
    else if (folderAbbr == "miscellaneous")
    {
        // Naming: Directly uses noteName (e.g., triC4-A4.wav, bassline1.wav)
        // Assume noteName from MML will directly map to filename (e.g., "triC4-A4", "bassline1")
        ss << noteName << ".wav";
        filename = ss.str();
    }
    else if (folderAbbr == "sk-5")
    {
        // Naming: <chorus_id>.wav (e.g., chorus1.wav, chorus2.wav)
        // Assume noteName from MML is "chorus1", "chorus2", etc.
        ss << noteName << ".wav";
        filename = ss.str();
    }
    else
    { // Pitched instruments: imp, i05, i25, sqr, tri
        // Naming: <canonicalNote><octave>.wav (e.g., A4.wav, Db5.wav)
        // For these, we call getCanonicalNoteFilename to build the note part.
        std::string canonicalNoteAndOctave = getCanonicalNoteFilename(noteName, accidental, octave);
        ss << canonicalNoteAndOctave << "-" << folderAbbr << ".wav"; // <--- MODIFIED LINE!
        filename = ss.str();
    }

    // Combine full path and filename
    return fullFolderPath + "/" + filename;
}

// --- calculateDurationFromLength Implementation ---
double NoteDecoder::calculateDurationFromLength(int length, double currentTempoBPM) const
{
    if (currentTempoBPM <= 0)
    {
        // Handle invalid tempo (e.g., throw an error, or return a default)
        // For robustness, let's throw an exception.
        throw std::invalid_argument("Tempo (BPM) must be greater than 0.");
    }
    if (length <= 0)
    {
        // Handle invalid note length (e.g., throw an error, or return a
        // default). A length of 0 or negative doesn't make musical sense.
        throw std::invalid_argument("Note length must be greater than 0.");
    }

    // A quarter note (length 4) is considered one "beat" at the given tempo.
    // Duration of one beat in seconds = 60.0 / currentTempoBPM
    double duration_of_one_beat = 60.0 / currentTempoBPM;

    // A whole note (length 1) is 4 beats.
    // So, the ratio for a given 'length' is (4.0 / length)
    // E.g., for length 4 (quarter note), ratio is 4.0/4 = 1
    // E.g., for length 8 (eighth note), ratio is 4.0/8 = 0.5
    // E.g., for length 1 (whole note), ratio is 4.0/1 = 4
    double note_length_ratio = 4.0 / static_cast<double>(length);

    return duration_of_one_beat * note_length_ratio;
}


std::string NoteDecoder::getCanonicalNoteFilename(
    const std::string &baseNote, // e.g., "A", "C", "D"
    char accidental,             // '+', '-', or ' ' (space/empty for natural)
    int octave                   // e.g., 4, 5
) const
{
    std::string canonicalNote = baseNote;

    // 1. Handle accidentals
    if (accidental == '+')
    {
        // Map sharps to their enharmonic flat equivalents
        // This map stores "sharp_note" -> "flat_equivalent"
        static const std::map<std::string, std::string> sharpToFlatOrNaturalMap = {
            {"C", "Db"}, {"D", "Eb"}, {"F", "Gb"}, {"G", "Ab"}, {"A", "Bb"},
            {"E", "F"}, // E# is F natural
            {"B", "C"}  // B# is C natural
        };

        auto it = sharpToFlatOrNaturalMap.find(baseNote);
        if (it != sharpToFlatOrNaturalMap.end())
        {
            canonicalNote = it->second;
        }
        else
        {
            // Should not happen for valid musical notes if map is exhaustive
            // Could throw an error here for invalid MML if desired
        }
    }
    else if (accidental == '-')
    {
        // Map flats to their enharmonic natural equivalents
        // if they are Cb or Fb.
        static const std::map<std::string, std::string> flatToNaturalMap = {
            {"C", "B"}, // Cb is B natural
            {"F", "E"}  // Fb is E natural
        };

        auto it = flatToNaturalMap.find(baseNote);
        if (it != flatToNaturalMap.end())
        {
            canonicalNote = it->second; // Use the natural equivalent
            // No 'b' appended here because it becomes a natural note
        }
        else
        {
            // For other flats, simply append 'b'
            canonicalNote += "b";
        }
    }
    // If accidental is ' ', it's a natural, so canonicalNote remains baseNote

    // 2. Append octave
    // Check if the folder type uses octave (pitched instruments)
    // For drums/noise, octave might be irrelevant but parsing expects it.
    // We only append octave for pitched instrument folders.
    // This logic would need to be in buildWaveformFilePath, or passed here.
    // For simplicity, let's assume this function *always* appends the octave
    // and the caller (buildWaveformFilePath) decides if it's used for the path.
    return canonicalNote + std::to_string(octave);
}


std::string NoteDecoder::getFullFolderPath(const std::string &folderAbbr) const
{
    if (folderAbbr == "imp")
        return "impulsewave";
    if (folderAbbr == "i05")
        return "impulse-05-wave";
    if (folderAbbr == "i25")
        return "impulse-25-wave";
    if (folderAbbr == "sqr")
        return "squarewave";
    if (folderAbbr == "tri")
        return "trianglewave";
    if (folderAbbr == "x")   // HOTFIX: all folder names are lowercase
        return "casio-drums";
    // For "noise", "miscellaneous", "sk-5", the abbreviation is the full name
    return folderAbbr;
}

int NoteDecoder::getLoadedSampleRate() const
{
    if (!m_sampleCache.empty())
    {
        // Return the sample rate of the first item in the cache
        // Assuming all your WAV files will have the same sample rate
        return m_sampleCache.begin()->second.sampleRate;
    }
    // Return a common default sample rate if no samples have been loaded yet
    return 44100; // Common sample rate for audio
}


// --- getNoteAudio Implementation ---
std::vector<float> NoteDecoder::getNoteAudio(
    const std::string &folderAbbr,
    const std::string &noteName,
    char accidental,
    int length,
    int octave,
    double explicitDurationSeconds,
    double currentTempoBPM)
{
    // 1. Build the full WAV file path
    std::string filePath = buildWaveformFilePath(
        folderAbbr, noteName, accidental, length, octave);

    // 2. Check cache for the sample
    SampleInfo loadedSample;
    auto cache_it = m_sampleCache.find(filePath);
    if (cache_it != m_sampleCache.end())
    {
        loadedSample = cache_it->second;
        std::cout << "Using cached WAV: " << filePath << std::endl; // For debugging
    }
    else
    {
        // Not in cache, load the file
        try
        {
            loadedSample = loadWavFile(filePath);
            m_sampleCache[filePath] = loadedSample; // Store in cache for future use
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error loading waveform for MML command ("
                      << folderAbbr << ":" << noteName << accidental << length << "o" << octave << "): "
                      << e.what() << std::endl;
            // Return an empty vector to indicate failure, or throw the exception up.
            // Returning empty allows the MML sequence to continue playing other notes.
            return {};
        }
    }

    // 3. Determine the target playback duration for this note
    double targetDurationSeconds = 0.0;
    if (explicitDurationSeconds > 0)
    {
        // Explicit duration from MML takes precedence
        targetDurationSeconds = explicitDurationSeconds;
    }
    else if (length > 0)
    {
        // If no explicit duration, use MML note length and tempo
        try
        {
            targetDurationSeconds = calculateDurationFromLength(length, currentTempoBPM);
        }
        catch (const std::invalid_argument &e)
        {
            std::cerr << "Error calculating duration for MML command ("
                      << folderAbbr << ":" << noteName << accidental << length << "o" << octave << "): "
                      << e.what() << std::endl;
            return {}; // Return empty on error
        }
    }
    else
    {
        // Fallback: If neither explicit duration nor length is provided,
        // use the original sample's natural duration. This is especially
        // useful for percussive sounds like drums and one-shot noises.
        targetDurationSeconds = loadedSample.durationSeconds;
        std::cout << "Using natural sample duration: " << targetDurationSeconds << "s" << std::endl; // For debugging
    }

    // --- ADD THIS DEBUG LINE ---
    std::cout << "Calculated target duration for " << folderAbbr << ":" << noteName
              << " (L=" << length << "): " << targetDurationSeconds << "s" << std::endl;
    // --- END DEBUG LINE ---

    // Ensure target duration is positive
    if (targetDurationSeconds <= 0)
    {
        std::cerr << "Warning: Calculated/explicit duration for MML command ("
                  << folderAbbr << ":" << noteName << accidental << length << "o" << octave << ") was non-positive ("
                  << targetDurationSeconds << "s). Returning empty audio." << std::endl;
        return {}; // Return empty audio if duration is invalid
    }

    // Handle looping vs. padding with silence based on instrument type
    std::vector<float> finalAudioData;
    size_t numSamplesToGenerate = static_cast<size_t>(targetDurationSeconds * loadedSample.sampleRate);

    // Convert folderAbbr to lowercase for comparison, assuming it's already
    // lowercased, but just for safety.
    std::string lowerFolderAbbr = folderAbbr;
    std::transform(lowerFolderAbbr.begin(), lowerFolderAbbr.end(), lowerFolderAbbr.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    // Define which folders should NOT loop, but rather pad with silence
    bool isOneShotInstrument = (lowerFolderAbbr == "x" || lowerFolderAbbr == "noise" ||
                                lowerFolderAbbr == "miscellaneous" || lowerFolderAbbr == "sk-5");

    if (isOneShotInstrument)
    {
        // For one-shot instruments (drums, effects):
        // Play the sample once, then pad with silence if desired_duration > sample_length
        finalAudioData.insert(finalAudioData.end(), loadedSample.data.begin(), loadedSample.data.end());

        if (finalAudioData.size() < numSamplesToGenerate)
        {
            // Pad with silence to reach the desired duration
            finalAudioData.resize(numSamplesToGenerate, 0.0f);
        }
        else if (finalAudioData.size() > numSamplesToGenerate)
        {
            // Truncate if the sample is longer than desired (less common for drums)
            finalAudioData.resize(numSamplesToGenerate);
        }
    }
    else // For pitched instruments (impulsewave, squarewave, trianglewave)
    {
        // Here, it's generally okay to loop the sample if desired_duration is longer.
        // So, you can use your existing generate_audio function for these.
        finalAudioData = generate_audio(loadedSample.data, loadedSample.sampleRate, targetDurationSeconds);
    }

    return finalAudioData;
}

// --- loadWavFile Implementation ---
SampleInfo NoteDecoder::loadWavFile(const std::string &filePath)
{
    SampleInfo info;
    SF_INFO sfinfo;
    SNDFILE *infile = nullptr;

    // Set file path in info struct for caching key
    info.filePath = filePath;

    // Open the input WAV file
    infile = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
    if (!infile)
    {
        throw std::runtime_error("Error opening WAV file: " + filePath + " - " + sf_strerror(nullptr));
    }

    // Check if the file is in a supported format (e.g., float, PCM)
    // For now, we assume all your files are fine for sf_read_float.
    // If sf_read_float fails, it usually means the file format isn't compatible.
    if (!sf_format_check(&sfinfo))
    {
        sf_close(infile);
        throw std::runtime_error("WAV file format not supported by libsndfile: " + filePath);
    }

    // Allocate buffer to hold audio data
    info.data.resize(sfinfo.frames * sfinfo.channels);

    // Read all frames from the WAV file into the buffer
    sf_count_t frames_read = sf_read_float(infile, info.data.data(), info.data.size());
    if (frames_read != sfinfo.frames)
    {
        // This is a warning, not necessarily an error, but worth noting.
        std::cerr << "Warning: Could not read all frames from " << filePath << ". Expected "
                  << sfinfo.frames << ", read " << frames_read << std::endl;
    }

    // Store sample rate and channels
    info.sampleRate = sfinfo.samplerate;
    info.channels = sfinfo.channels;

    // Calculate the original duration of the sample
    if (sfinfo.samplerate > 0 && sfinfo.channels > 0)
    {
        info.durationSeconds = static_cast<double>(info.data.size()) / (sfinfo.samplerate * sfinfo.channels);
    }
    else
    {
        info.durationSeconds = 0.0; // Should not happen with valid WAV files
    }

    // Close the input file
    sf_close(infile);

    std::cout << "Successfully loaded WAV: " << filePath
              << " (Rate: " << info.sampleRate
              << ", Ch: " << info.channels
              << ", Dur: " << info.durationSeconds << "s)" << std::endl;

    return info;
}
