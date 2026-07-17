#pragma once

#include "engine/Log.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace bunker
{
    struct TextureGenConfig
    {
        std::string baseTexturePath;
        std::string maskPath;
        std::string outputDir;
        std::string prefix;
        int variantCount = 4;
        std::string method = "recolor";
        bool generateNormalMap = false;
        float normalStrength = 1.0f;
    };

    class TextureGenerator
    {
      private:
        std::map<std::string, sf::Image> m_BaseCache;

        static float perlinNoise(float x, float y, int seed = 0);
        static float fbm(float x, float y, int octaves, int seed = 0);
        static std::uint8_t clampByte(int v);

      public:
        TextureGenerator() = default;

        bool needsGeneration(const std::string& outputDir, int expectedCount) const;
        bool generate(const TextureGenConfig& cfg);
        void generateAllOnFirstRun();

      private:
        bool loadBase(const std::string& path, sf::Image& out);
        static void applyRecolor(sf::Image& img, const sf::Image& mask, bool hasMask, int seed);
        static void applyTerrainNoise(sf::Image& img, int seed);
        static void applyCreatureVariation(sf::Image& img, const sf::Image& mask, bool hasMask, int seed);
        static void applyRobotScheme(sf::Image& img, const sf::Image& mask, bool hasMask, int seed);
        static void applyItemVariation(sf::Image& img, const sf::Image& mask, bool hasMask, int seed);
        static sf::Image generateNormalMap(const sf::Image& source, float strength);
    };

} // namespace bunker

#include "content/TextureGeneratorCore.inl"