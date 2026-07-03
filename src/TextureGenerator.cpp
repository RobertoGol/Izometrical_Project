#include "TextureGenerator.hpp"
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

bool TextureGenerator::generate(const TextureGenConfig &cfg)
{
    if (!fs::exists(cfg.inputPath))
    {
        std::cerr << "Input file not found: " << cfg.inputPath << std::endl;
        return false;
    }

    // Создаём папку вывода, если её нет
    fs::path outputDir = fs::path(cfg.outputPath).parent_path();
    if (!outputDir.empty() && !fs::exists(outputDir))
    {
        fs::create_directories(outputDir);
    }

    std::cout << "[TextureGenerator] Generating texture from: " << cfg.inputPath << std::endl;

    // TODO: Здесь будет реальная логика наложения масок и генерации
    // Пока просто копируем/заглушка

    return true;
}