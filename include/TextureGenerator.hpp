#pragma once

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

struct TextureGenConfig
{
    std::string inputPath;
    std::string maskPath;
    std::string outputPath;
};

class TextureGenerator
{
public:
    bool generate(const TextureGenConfig &cfg);

private:
    bool fileExists(const std::string &path) const
    {
        return fs::exists(path);
    }
};