#pragma once

#include "engine/Log.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // Процедурный запекатель тайловых атласов Убежища 17 (Omniverse v.133)
    // Упаковывает сотни сгенерированных тайлов в единую мега-текстуру для слабых ПК
    // ═══════════════════════════════════════════════════════════════════════════════

    struct AtlasSubRect
    {
        int x = 0, y = 0, w = 16, h = 16;
    };

    class MegaTileAtlasBaker
    {
      public:
        static bool bakeTileAtlas(const std::vector<sf::Image>& tiles, int atlasWidth, sf::Image& outAtlasImg,
                                  std::vector<AtlasSubRect>& outRects)
        {
            if (tiles.empty() || atlasWidth <= 0)
                return false;
            int tileW = tiles[0].getSize().x;
            int tileH = tiles[0].getSize().y;
            if (tileW <= 0 || tileH <= 0)
                return false;

            int cols = atlasWidth / tileW;
            if (cols <= 0)
                cols = 1;
            int rows = (static_cast<int>(tiles.size()) + cols - 1) / cols;
            int atlasHeight = rows * tileH;

            outAtlasImg.resize({static_cast<unsigned int>(atlasWidth), static_cast<unsigned int>(atlasHeight)},
                               sf::Color::Transparent);
            outRects.clear();

            for (std::size_t i = 0; i < tiles.size(); ++i)
            {
                int c = static_cast<int>(i) % cols;
                int r = static_cast<int>(i) / cols;
                int px = c * tileW;
                int py = r * tileH;

                (void)outAtlasImg.copy(tiles[i], {static_cast<unsigned int>(px), static_cast<unsigned int>(py)});
                outRects.push_back({px, py, tileW, tileH});
            }

            bunker::logInfo() << "[TEXGEN] Запечен Мега-Атлас Убежища 17 (" << atlasWidth << "x" << atlasHeight
                              << "), упаковано тайлов: " << tiles.size() << " (0% спайков I/O)!" << std::endl;
            return true;
        }
    };


} // namespace bunker
