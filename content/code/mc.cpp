#include <iostream>
#include <fstream>
#include <vector>
#include <sndfile.h>
#include <stdexcept>

// Looping function
std::vector<float> generate_audio(const std::vector<float> &sample_data, double sample_rate, double desired_duration)
{
    std::vector<float> output_audio;
    double sample_length_seconds = static_cast<double>(sample_data.size()) / sample_rate;

    if (desired_duration <= 0)
    {
        return output_audio; // Return empty if no duration or negative duration
    }

    if (desired_duration <= sample_length_seconds)
    {
        // If the desired duration is shorter than the sample, we take a segment
        size_t num_samples_to_take = static_cast<size_t>(desired_duration * sample_rate);
        output_audio.insert(output_audio.end(), sample_data.begin(), sample_data.begin() + num_samples_to_take);
    }
    else
    {
        // If the desired duration is longer, we loop the sample
        double num_loops_exact = desired_duration / sample_length_seconds;
        size_t num_full_loops = static_cast<size_t>(num_loops_exact);
        double remaining_duration = desired_duration - (num_full_loops * sample_length_seconds);
        size_t num_remaining_samples = static_cast<size_t>(remaining_duration * sample_rate);

        // Add the full loops
        for (size_t i = 0; i < num_full_loops; ++i)
        {
            output_audio.insert(output_audio.end(), sample_data.begin(), sample_data.end());
        }

        // Add the remaining part of the sample (if any)
        if (num_remaining_samples > 0)
        {
            output_audio.insert(output_audio.end(), sample_data.begin(), sample_data.begin() + num_remaining_samples);
        }
    }

    return output_audio;
}


int main()
{
    const char *input_filename = "A4-sqr.wav";
    const char *output_filename = "sine_wave.pcm"; // Still using this name

    SNDFILE *infile = nullptr;
    SF_INFO sfinfo;

    // Open the input WAV file
    infile = sf_open(input_filename, SFM_READ, &sfinfo);
    if (!infile)
    {
        std::cerr << "Error opening input file: " << input_filename << " - " << sf_strerror(nullptr) << std::endl;
        return 1;
    }

    // Print some info about the WAV file
    std::cout << "Opened input file: " << input_filename << std::endl;
    std::cout << "  Sample rate: " << sfinfo.samplerate << std::endl;
    std::cout << "  Channels: " << sfinfo.channels << std::endl;
    std::cout << "  Frames: " << sfinfo.frames << std::endl;
    std::cout << "  Format: 0x" << std::hex << sfinfo.format << std::dec << std::endl;

    // Read all frames from the WAV file into a buffer
    std::vector<float> sample_data(sfinfo.frames * sfinfo.channels);
    sf_count_t frames_read = sf_read_float(infile, sample_data.data(), sample_data.size());
    if (frames_read != sfinfo.frames)
    {
        std::cerr << "Warning: Could not read all frames from input file." << std::endl;
    }

    // Close the input file
    sf_close(infile);

    // Define the desired duration (5 seconds)
    double desired_duration = 5.0;

    // Generate the audio data for the desired duration by looping
    std::vector<float> output_audio = generate_audio(sample_data, static_cast<double>(sfinfo.samplerate), desired_duration);

    // Open the output PCM file
    std::ofstream outfile(output_filename, std::ios::binary);
    if (!outfile)
    {
        std::cerr << "Error opening output file: " << output_filename << std::endl;
        return 1;
    }

    // Write the generated audio data to the PCM file
    outfile.write(reinterpret_cast<const char *>(output_audio.data()), output_audio.size() * sizeof(float));

    outfile.close();
    std::cout << "Looped audio data (5 seconds) written to " << output_filename << " (raw float)." << std::endl;

    return 0;
}
