#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <functional>
#include <map>

namespace bunker {

// ═══════════════════════════════════════════════════════
// Система процедурной генерации текстур
//
// Идея: есть БАЗОВАЯ текстура (PNG), на её основе
// генератор создаёт вариации разными методами.
// Генерируется ОДИН РАЗ при первом запуске,
// результат запекается в assets/generated/ и больше
// не пересоздаётся.
//
// Типы генерации:
//   - terrain    : земля, камни, трава (шум Перлина + вариации)
//   - creature   : мобы/зомби (с маской лица/тела)
//   - robot      : роботы (цветовые схемы + паттерны)
//   - item       : оружие/вещи (перекраска + детали)
//   - normal_map : карта нормалей для выпуклостей
// ═══════════════════════════════════════════════════════

// ── Маска: какие зоны текстуры менять, какие оставить ──
// Чёрный пиксель в маске = НЕ ТРОГАТЬ (лицо, глаза)
// Белый пиксель = можно менять (тело, одежда)
// Серый = частично менять (смешивание)

struct TextureGenConfig {
    std::string baseTexturePath;      // Базовая текстура
    std::string maskPath;             // Маска (опционально, "" = менять всё)
    std::string outputDir;            // Папка для сгенерированных
    std::string prefix;               // Префикс файлов (enemy_01, ground_03...)
    int         variantCount = 4;     // Сколько вариаций создать
    std::string method = "recolor";   // Метод генерации
    bool        generateNormalMap = false;  // Генерировать карту нормалей?
    float       normalStrength = 1.0f;     // Сила выпуклостей
};

class TextureGenerator {
private:
    // Кэш уже загруженных базовых текстур
    std::map<std::string, sf::Image> m_BaseCache;

    // ── Шум Перлина (упрощённый) ──
    static float perlinNoise(float x, float y, int seed = 0) {
        // Простой value noise с интерполяцией
        auto hash = [](int xi, int yi, int s) -> float {
            int n = xi * 374761393 + yi * 668265263 + s;
            n = (n << 13) ^ n;
            return 1.0f - static_cast<float>((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        };

        int xi = static_cast<int>(std::floor(x));
        int yi = static_cast<int>(std::floor(y));
        float xf = x - xi;
        float yf = y - yi;

        // Smooth step
        float u = xf * xf * (3.0f - 2.0f * xf);
        float v = yf * yf * (3.0f - 2.0f * yf);

        float n00 = hash(xi, yi, seed);
        float n10 = hash(xi + 1, yi, seed);
        float n01 = hash(xi, yi + 1, seed);
        float n11 = hash(xi + 1, yi + 1, seed);

        float nx0 = n00 * (1 - u) + n10 * u;
        float nx1 = n01 * (1 - u) + n11 * u;

        return nx0 * (1 - v) + nx1 * v;
    }

    // ── Fractal Brownian Motion (несколько октав шума) ──
    static float fbm(float x, float y, int octaves, int seed = 0) {
        float value = 0.0f;
        float amplitude = 0.5f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; ++i) {
            value += amplitude * perlinNoise(x * frequency, y * frequency, seed + i * 137);
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }
        return value;
    }

    // ── Clamp байт ──
    static sf::Uint8 clampByte(int v) {
        return static_cast<sf::Uint8>(std::max(0, std::min(255, v)));
    }

public:
    TextureGenerator() = default;

    // ═══════════════════════════════════════════════
    // Проверить, нужна ли генерация
    // (если папка generated/ уже содержит файлы — пропускаем)
    // ═══════════════════════════════════════════════
    bool needsGeneration(const std::string& outputDir, int expectedCount) const {
        if (!std::filesystem::exists(outputDir)) return true;

        int count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(outputDir)) {
            if (entry.path().extension() == ".png") count++;
        }
        return count < expectedCount;
    }

    // ═══════════════════════════════════════════════
    // Главная точка входа: генерация по конфигу
    // ═══════════════════════════════════════════════
    bool generate(const TextureGenConfig& cfg) {
        // Проверяем, нужно ли генерировать
        if (!needsGeneration(cfg.outputDir, cfg.variantCount)) {
            std::cout << "[TEXGEN] Пропуск " << cfg.outputDir << " — уже запечено." << std::endl;
            return true;
        }

        // Загружаем базовую текстуру
        sf::Image baseImg;
        if (!loadBase(cfg.baseTexturePath, baseImg)) {
            std::cerr << "[TEXGEN] Базовая текстура не найдена: " << cfg.baseTexturePath << std::endl;
            return false;
        }

        // Загружаем маску (если есть)
        sf::Image maskImg;
        bool hasMask = false;
        if (!cfg.maskPath.empty() && maskImg.loadFromFile(cfg.maskPath)) {
            hasMask = true;
        }

        // Создаём выходную папку
        std::filesystem::create_directories(cfg.outputDir);

        // Генерируем вариации
        for (int i = 0; i < cfg.variantCount; ++i) {
            sf::Image variant = baseImg;  // Копия базы
            int seed = static_cast<int>(std::time(nullptr)) + i * 7919;

            if (cfg.method == "recolor") {
                applyRecolor(variant, maskImg, hasMask, seed);
            } else if (cfg.method == "terrain") {
                applyTerrainNoise(variant, seed);
            } else if (cfg.method == "creature") {
                applyCreatureVariation(variant, maskImg, hasMask, seed);
            } else if (cfg.method == "robot") {
                applyRobotScheme(variant, maskImg, hasMask, seed);
            } else if (cfg.method == "item") {
                applyItemVariation(variant, maskImg, hasMask, seed);
            }

            // Сохраняем вариацию
            std::string outPath = cfg.outputDir + "/" + cfg.prefix + "_"
                                + std::to_string(i) + ".png";
            variant.saveToFile(outPath);

            // Генерация карты нормалей (для визуальных выпуклостей)
            if (cfg.generateNormalMap) {
                sf::Image normalMap = generateNormalMap(variant, cfg.normalStrength);
                std::string normalPath = cfg.outputDir + "/" + cfg.prefix + "_"
                                       + std::to_string(i) + "_normal.png";
                normalMap.saveToFile(normalPath);
            }

            std::cout << "[TEXGEN] Сгенерировано: " << outPath << std::endl;
        }

        return true;
    }

    // ═══════════════════════════════════════════════
    // Пакетная генерация всех текстур при первом запуске
    // ═══════════════════════════════════════════════
    void generateAllOnFirstRun() {
        std::cout << "[TEXGEN] Проверка текстур при запуске..." << std::endl;

        // ── Земля / тайлы ──
        if (std::filesystem::exists("assets/textures/base_ground.png")) {
            generate({
                "assets/textures/base_ground.png",
                "",  // Без маски
                "assets/generated/ground",
                "ground",
                8,       // 8 вариаций
                "terrain",
                true,    // Генерировать normal map
                1.5f     // Сила выпуклостей
            });
        }

        // ── Стены ──
        if (std::filesystem::exists("assets/textures/base_wall.png")) {
            generate({
                "assets/textures/base_wall.png", "",
                "assets/generated/walls", "wall", 4, "terrain", true, 2.0f
            });
        }

        // ── Враги / мобы ──
        if (std::filesystem::exists("assets/textures/base_enemy.png")) {
            generate({
                "assets/textures/base_enemy.png",
                "assets/textures/base_enemy_mask.png",  // Маска: лицо не трогаем!
                "assets/generated/enemies", "enemy", 6, "creature", true, 1.0f
            });
        }

        // ── Роботы ──
        if (std::filesystem::exists("assets/textures/base_robot.png")) {
            generate({
                "assets/textures/base_robot.png",
                "assets/textures/base_robot_mask.png",
                "assets/generated/robots", "robot", 5, "robot", true, 2.5f
            });
        }

        // ── Оружие / предметы ──
        if (std::filesystem::exists("assets/textures/base_item.png")) {
            generate({
                "assets/textures/base_item.png",
                "assets/textures/base_item_mask.png",
                "assets/generated/items", "item", 10, "item", true, 1.2f
            });
        }

        std::cout << "[TEXGEN] Генерация текстур завершена." << std::endl;
    }

private:
    // ── Загрузка базовой текстуры с кэшем ──
    bool loadBase(const std::string& path, sf::Image& out) {
        auto it = m_BaseCache.find(path);
        if (it != m_BaseCache.end()) {
            out = it->second;
            return true;
        }
        if (!out.loadFromFile(path)) return false;
        m_BaseCache[path] = out;
        return true;
    }

    // ═══════════════════════════════════════════════
    // МЕТОД: Перекраска (recolor)
    // Сдвиг оттенка + насыщенности с учётом маски
    // ═══════════════════════════════════════════════
    void applyRecolor(sf::Image& img, const sf::Image& mask, bool hasMask, int seed) {
        unsigned w = img.getSize().x;
        unsigned h = img.getSize().y;

        float hueShift = static_cast<float>(seed % 360);
        float satMod   = 0.7f + static_cast<float>(seed % 60) / 100.0f;
        float valMod   = 0.85f + static_cast<float>(seed % 30) / 100.0f;

        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; ++x) {
                // Проверяем маску
                if (hasMask && x < mask.getSize().x && y < mask.getSize().y) {
                    sf::Color m = mask.getPixel(x, y);
                    if (m.r < 30 && m.g < 30 && m.b < 30) continue;  // Чёрный = не трогать
                }

                sf::Color c = img.getPixel(x, y);
                if (c.a == 0) continue;

                // RGB → HSV → сдвиг → RGB
                float r = c.r / 255.0f, g = c.g / 255.0f, b = c.b / 255.0f;
                float cmax = std::max({r, g, b}), cmin = std::min({r, g, b});
                float delta = cmax - cmin;

                float hue = 0, sat = 0, val = cmax;
                if (delta > 0.001f) {
                    sat = delta / cmax;
                    if (cmax == r) hue = 60.0f * std::fmod((g - b) / delta, 6.0f);
                    else if (cmax == g) hue = 60.0f * ((b - r) / delta + 2.0f);
                    else hue = 60.0f * ((r - g) / delta + 4.0f);
                    if (hue < 0) hue += 360.0f;
                }

                hue = std::fmod(hue + hueShift, 360.0f);
                sat = std::min(1.0f, sat * satMod);
                val = std::min(1.0f, val * valMod);

                // HSV → RGB
                float C = val * sat;
                float X = C * (1 - std::abs(std::fmod(hue / 60.0f, 2.0f) - 1));
                float m2 = val - C;
                float r2 = 0, g2 = 0, b2 = 0;

                if (hue < 60)       { r2 = C; g2 = X; }
                else if (hue < 120) { r2 = X; g2 = C; }
                else if (hue < 180) { g2 = C; b2 = X; }
                else if (hue < 240) { g2 = X; b2 = C; }
                else if (hue < 300) { r2 = X; b2 = C; }
                else                { r2 = C; b2 = X; }

                img.setPixel(x, y, sf::Color(
                    clampByte(static_cast<int>((r2 + m2) * 255)),
                    clampByte(static_cast<int>((g2 + m2) * 255)),
                    clampByte(static_cast<int>((b2 + m2) * 255)),
                    c.a
                ));
            }
        }
    }

    // ═══════════════════════════════════════════════
    // МЕТОД: Террейн (terrain)
    // Шум Перлина + цветовые вариации + пятна
    // ═══════════════════════════════════════════════
    void applyTerrainNoise(sf::Image& img, int seed) {
        unsigned w = img.getSize().x;
        unsigned h = img.getSize().y;

        float scale = 0.05f + static_cast<float>(seed % 10) / 100.0f;

        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; ++x) {
                sf::Color c = img.getPixel(x, y);
                if (c.a == 0) continue;

                float noise = fbm(x * scale, y * scale, 4, seed);
                // noise в диапазоне примерно -1..1, приводим к 0.7..1.3
                float factor = 1.0f + noise * 0.3f;

                // Добавляем мелкий гранулярный шум
                float grain = perlinNoise(x * 0.3f, y * 0.3f, seed + 999) * 0.1f;
                factor += grain;

                img.setPixel(x, y, sf::Color(
                    clampByte(static_cast<int>(c.r * factor)),
                    clampByte(static_cast<int>(c.g * factor)),
                    clampByte(static_cast<int>(c.b * factor)),
                    c.a
                ));
            }
        }
    }

    // ═══════════════════════════════════════════════
    // МЕТОД: Существо (creature)
    // Перекраска тела с сохранением лица через маску
    // + пятна/полосы/раны
    // ═══════════════════════════════════════════════
    void applyCreatureVariation(sf::Image& img, const sf::Image& mask, bool hasMask, int seed) {
        // Сначала перекрашиваем (с маской)
        applyRecolor(img, mask, hasMask, seed);

        unsigned w = img.getSize().x;
        unsigned h = img.getSize().y;

        // Добавляем случайные пятна/полосы
        int spotCount = 3 + seed % 8;
        for (int i = 0; i < spotCount; ++i) {
            int cx = (seed * (i + 1) * 7) % w;
            int cy = (seed * (i + 1) * 13) % h;
            int radius = 2 + (seed * (i + 1)) % 6;
            float darkness = 0.6f + static_cast<float>(seed % 40) / 100.0f;

            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int px = cx + dx, py = cy + dy;
                    if (px < 0 || py < 0 || px >= (int)w || py >= (int)h) continue;
                    if (dx * dx + dy * dy > radius * radius) continue;

                    // Не трогаем маскированные зоны (лицо)
                    if (hasMask && px < (int)mask.getSize().x && py < (int)mask.getSize().y) {
                        sf::Color m = mask.getPixel(px, py);
                        if (m.r < 30) continue;
                    }

                    sf::Color c = img.getPixel(px, py);
                    if (c.a == 0) continue;

                    float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy)) / radius;
                    float blend = (1.0f - dist) * (1.0f - darkness);

                    img.setPixel(px, py, sf::Color(
                        clampByte(static_cast<int>(c.r * (1.0f - blend * 0.4f))),
                        clampByte(static_cast<int>(c.g * (1.0f - blend * 0.3f))),
                        clampByte(static_cast<int>(c.b * (1.0f - blend * 0.2f))),
                        c.a
                    ));
                }
            }
        }
    }

    // ═══════════════════════════════════════════════
    // МЕТОД: Робот (robot)
    // Цветовые схемы + металлические паттерны + царапины
    // ═══════════════════════════════════════════════
    void applyRobotScheme(sf::Image& img, const sf::Image& mask, bool hasMask, int seed) {
        // Цветовая схема (выбираем одну из N)
        struct ColorScheme { float r, g, b; };
        ColorScheme schemes[] = {
            {0.8f, 0.2f, 0.1f},   // Красный
            {0.2f, 0.3f, 0.8f},   // Синий
            {0.7f, 0.7f, 0.1f},   // Жёлтый
            {0.1f, 0.7f, 0.3f},   // Зелёный
            {0.5f, 0.5f, 0.5f},   // Серый
            {0.9f, 0.5f, 0.0f},   // Оранжевый
            {0.3f, 0.1f, 0.6f},   // Фиолетовый
        };
        ColorScheme cs = schemes[seed % 7];

        unsigned w = img.getSize().x;
        unsigned h = img.getSize().y;

        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; ++x) {
                if (hasMask && x < mask.getSize().x && y < mask.getSize().y) {
                    sf::Color m = mask.getPixel(x, y);
                    if (m.r < 30) continue;  // Сенсоры/глаза не трогать
                }

                sf::Color c = img.getPixel(x, y);
                if (c.a == 0) continue;

                float gray = (c.r + c.g + c.b) / (3.0f * 255.0f);

                // Металлический блеск (шум)
                float metallic = perlinNoise(x * 0.1f, y * 0.1f, seed) * 0.15f;
                gray += metallic;

                img.setPixel(x, y, sf::Color(
                    clampByte(static_cast<int>(gray * cs.r * 255)),
                    clampByte(static_cast<int>(gray * cs.g * 255)),
                    clampByte(static_cast<int>(gray * cs.b * 255)),
                    c.a
                ));
            }
        }

        // Царапины
        int scratchCount = 2 + seed % 5;
        for (int i = 0; i < scratchCount; ++i) {
            int sx = (seed * (i + 3)) % w;
            int sy = (seed * (i + 7)) % h;
            int length = 5 + seed % 15;
            float angle = static_cast<float>(seed * i) * 0.1f;

            for (int j = 0; j < length; ++j) {
                int px = sx + static_cast<int>(j * std::cos(angle));
                int py = sy + static_cast<int>(j * std::sin(angle));
                if (px < 0 || py < 0 || px >= (int)w || py >= (int)h) continue;

                sf::Color c = img.getPixel(px, py);
                img.setPixel(px, py, sf::Color(
                    clampByte(c.r + 40), clampByte(c.g + 40), clampByte(c.b + 40), c.a
                ));
            }
        }
    }

    // ═══════════════════════════════════════════════
    // МЕТОД: Предмет (item)
    // Перекраска деталей + мелкие изменения формы
    // ═══════════════════════════════════════════════
    void applyItemVariation(sf::Image& img, const sf::Image& mask, bool hasMask, int seed) {
        applyRecolor(img, mask, hasMask, seed);

        unsigned w = img.getSize().x;
        unsigned h = img.getSize().y;

        // Лёгкое изменение яркости по зонам
        float brightZone = static_cast<float>(seed % 100) / 100.0f;
        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; ++x) {
                sf::Color c = img.getPixel(x, y);
                if (c.a == 0) continue;

                float yFactor = static_cast<float>(y) / static_cast<float>(h);
                float mod = 0.9f + yFactor * 0.2f * brightZone;

                img.setPixel(x, y, sf::Color(
                    clampByte(static_cast<int>(c.r * mod)),
                    clampByte(static_cast<int>(c.g * mod)),
                    clampByte(static_cast<int>(c.b * mod)),
                    c.a
                ));
            }
        }
    }

    // ═══════════════════════════════════════════════
    // Генерация карты нормалей из текстуры
    // Создаёт РЕАЛЬНЫЕ выпуклости, не нарисованные!
    // Используется Sobel-фильтр по яркости пикселей.
    // ═══════════════════════════════════════════════
    sf::Image generateNormalMap(const sf::Image& source, float strength) {
        unsigned w = source.getSize().x;
        unsigned h = source.getSize().y;

        sf::Image normalMap;
        normalMap.create(w, h, sf::Color(128, 128, 255));  // Нейтральная нормаль (0,0,1)

        auto getGray = [&](int x, int y) -> float {
            x = std::max(0, std::min(static_cast<int>(w) - 1, x));
            y = std::max(0, std::min(static_cast<int>(h) - 1, y));
            sf::Color c = source.getPixel(x, y);
            return (c.r + c.g + c.b) / (3.0f * 255.0f);
        };

        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; ++x) {
                // Sobel X
                float dx = -getGray(x-1, y-1) - 2*getGray(x-1, y) - getGray(x-1, y+1)
                          + getGray(x+1, y-1) + 2*getGray(x+1, y) + getGray(x+1, y+1);
                // Sobel Y
                float dy = -getGray(x-1, y-1) - 2*getGray(x, y-1) - getGray(x+1, y-1)
                          + getGray(x-1, y+1) + 2*getGray(x, y+1) + getGray(x+1, y+1);

                dx *= strength;
                dy *= strength;

                // Нормализуем вектор нормали
                float dz = 1.0f;
                float len = std::sqrt(dx * dx + dy * dy + dz * dz);
                dx /= len;
                dy /= len;
                dz /= len;

                // Кодируем в RGB: [-1,1] → [0,255]
                normalMap.setPixel(x, y, sf::Color(
                    clampByte(static_cast<int>((dx * 0.5f + 0.5f) * 255)),
                    clampByte(static_cast<int>((dy * 0.5f + 0.5f) * 255)),
                    clampByte(static_cast<int>((dz * 0.5f + 0.5f) * 255)),
                    255
                ));
            }
        }

        return normalMap;
    }
};

}  // namespace bunker
