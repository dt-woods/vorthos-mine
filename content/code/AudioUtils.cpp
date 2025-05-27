// Add this function to your main.cpp or a suitable utility file

#include "AudioUtils.h"
#include <fstream> // Required for file operations
#include <vector>   // Required for std::vector
#include <iostream> // For error messages
#include <sstream>  // Required for std::stringstream
#include <string>   // Required for std::string
#include <iostream> // For std::cerr, std::cout

// Function to read the entire content of a file into a single string
std::string readFileIntoString(const std::string &filePath)
{
    std::ifstream inputFileStream(filePath);

    if (!inputFileStream.is_open())
    {
        std::cerr << "Error: Could not open MML file: " << filePath << std::endl;
        return ""; // Return an empty string to indicate failure
    }

    // Read the entire file content into a stringstream
    std::stringstream buffer;
    buffer << inputFileStream.rdbuf();

    inputFileStream.close(); // Close the file stream

    return buffer.str(); // Return the string content
}


// Function to save a vector of float audio samples to a raw PCM file
bool saveToPcmFile(const std::vector<float> &audioData, const std::string &filename)
{
    std::ofstream outFile(filename, std::ios::out | std::ios::binary);

    if (!outFile.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return false;
    }

    // Write the raw float data directly to the file
    // std::vector<float>::data() gives a pointer to the underlying array
    // sizeof(float) * audioData.size() gives the total number of bytes
    outFile.write(reinterpret_cast<const char *>(audioData.data()), audioData.size() * sizeof(float));

    if (outFile.fail())
    {
        std::cerr << "Error: Failed to write audio data to " << filename << "." << std::endl;
        outFile.close();
        return false;
    }

    outFile.close();
    std::cout << "Successfully wrote " << audioData.size() << " float samples to " << filename << std::endl;
    return true;
}
