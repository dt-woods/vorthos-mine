#include <string>
#include <vector> // Required for std::vector

std::string readFileIntoString(const std::string &filePath);

bool saveToPcmFile(const std::vector<float> &audioData, const std::string &filename);
