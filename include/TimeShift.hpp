#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Constants.hpp"
#include "EnemySpawner.hpp"
#include <array>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace bunker {

// ═══════════════════════════════════════════════════════
// TimeShift — механика переноса во времени (Titanfall 2,
// миссия "Effect and Cause")
//
// Две временные линии:
//   PRESENT — настоящее (разрушенное Убежище 17, Рой Верминов)
//   PAST    — прошлое (Убежище до катастрофы, другие враги/роботы)
//
// Переключение по кнопке (T). Мир мгновенно меняется:
//   - Карта (стены, проходы) — разная в двух таймлайнах
//   - Враги — разные в каждом времени
//   - Эрозия — только в настоящем
//   - Визуальный эффект (мерцание, цветовой сдвиг)
//
// Игрок может застрять при переключении если в новом
// таймлайне на его месте стена → выталкивание / отмена.
//
// Кулдаун между переключениями.
// Заряд устройства ограничен — нельзя сидеть вечно в прошлом.
// ═══════════════════════════════════════════════════════

enum class Timeline {
    Present,   // Настоящее — разрушенное, Рой, эрозия
    Past       // Прошлое — целое, роботы-охранники, нет эрозии
};

struct TimelineSnapshot {
    // Карта тайлов для этого таймлайна
    std::array<std::array<int, Config::MAP_HEIGHT>, Config::MAP_WIDTH> sectorMap{};
    std::array<std::array<int, Config::MAP_HEIGHT>, Config::MAP_WIDTH> wallDurability{};
    std::array<std::array<float, Config::MAP_HEIGHT>, Config::MAP_WIDTH> erosionMap{};

    // Враги этого таймлайна (замораживаются при переключении)
    std::vector<Enemy> enemies;

    // Лут-контейнеры
    std::vector<LootContainer> lootContainers;
};

class TimeShift {
private:
    Timeline m_CurrentTimeline = Timeline::Present;

    // Снимки двух таймлайнов
    TimelineSnapshot m_PresentSnapshot;
    TimelineSnapshot m_PastSnapshot;

    // Кулдаун между переключениями
    float m_ShiftCooldown = 0.0f;
    float m_ShiftCooldownMax = 1.5f;  // 1.5 секунды между переключениями

    // Заряд устройства (ограниченное время в прошлом)
    float m_DeviceCharge = 100.0f;
    float m_MaxCharge = 100.0f;
    float m_DrainRate = 8.0f;      // Разряд в прошлом (за секунду)
    float m_RechargeRate = 5.0f;   // Подзарядка в настоящем

    // Визуальный эффект при переключении
    float m_TransitionTimer = 0.0f;
    float m_TransitionDuration = 0.4f;
    bool  m_IsTransitioning = false;

    bool m_Initialized = false;

public:
    TimeShift() = default;

    // ═══════════════════════════════════════════════
    // Инициализация двух таймлайнов
    // Вызывать после generateDefaultWorld()
    // ═══════════════════════════════════════════════
    void initialize(GameState& gs, EnemySpawner& spawner) {
        // ── Сохраняем настоящее (текущее состояние мира) ──
        savePresentFromGameState(gs);

        // ── Генерируем прошлое (другая карта) ──
        generatePastTimeline(gs, spawner);

        m_CurrentTimeline = Timeline::Present;
        m_Initialized = true;

        std::cout << "[TIMESHIFT] Два таймлайна инициализированы." << std::endl;
    }

    // ═══════════════════════════════════════════════
    // Попытка переключения (вызывать по нажатию T)
    // ═══════════════════════════════════════════════
    bool tryShift(GameState& gs) {
        if (!m_Initialized) return false;
        if (m_ShiftCooldown > 0.0f) return false;
        if (m_IsTransitioning) return false;

        // Нельзя переключиться в прошлое если заряд кончился
        if (m_CurrentTimeline == Timeline::Present && m_DeviceCharge < 5.0f) {
            std::cout << "[TIMESHIFT] Устройство разряжено!" << std::endl;
            return false;
        }

        // Сохраняем текущий таймлайн
        if (m_CurrentTimeline == Timeline::Present) {
            savePresentFromGameState(gs);
        } else {
            savePastFromGameState(gs);
        }

        // Переключаем
        Timeline newTimeline = (m_CurrentTimeline == Timeline::Present)
                               ? Timeline::Past : Timeline::Present;

        // Проверяем: не застрянет ли игрок в стене нового таймлайна?
        const auto& newMap = (newTimeline == Timeline::Present)
                             ? m_PresentSnapshot.sectorMap
                             : m_PastSnapshot.sectorMap;

        int px = static_cast<int>(gs.playerPos.x);
        int py = static_cast<int>(gs.playerPos.y);

        if (px >= 0 && px < Config::MAP_WIDTH && py >= 0 && py < Config::MAP_HEIGHT) {
            if (newMap[px][py] == 1) {
                // Попытка найти свободную ячейку рядом
                bool found = false;
                for (int dx = -1; dx <= 1 && !found; ++dx) {
                    for (int dy = -1; dy <= 1 && !found; ++dy) {
                        int nx = px + dx, ny = py + dy;
                        if (nx >= 0 && nx < Config::MAP_WIDTH &&
                            ny >= 0 && ny < Config::MAP_HEIGHT) {
                            if (newMap[nx][ny] != 1) {
                                gs.playerPos.x = static_cast<float>(nx) + 0.5f;
                                gs.playerPos.y = static_cast<float>(ny) + 0.5f;
                                found = true;
                            }
                        }
                    }
                }
                if (!found) {
                    std::cout << "[TIMESHIFT] Невозможно переключиться — застрянем в стене!" << std::endl;
                    return false;
                }
            }
        }

        // Загружаем новый таймлайн в GameState
        if (newTimeline == Timeline::Present) {
            loadPresentToGameState(gs);
        } else {
            loadPastToGameState(gs);
        }

        m_CurrentTimeline = newTimeline;
        m_ShiftCooldown = m_ShiftCooldownMax;
        m_IsTransitioning = true;
        m_TransitionTimer = m_TransitionDuration;

        std::string name = (newTimeline == Timeline::Present) ? "НАСТОЯЩЕЕ" : "ПРОШЛОЕ";
        std::cout << "[TIMESHIFT] Переход в " << name << std::endl;

        return true;
    }

    // ═══════════════════════════════════════════════
    // Обновление каждый кадр
    // ═══════════════════════════════════════════════
    void update(float dt) {
        // Кулдаун
        if (m_ShiftCooldown > 0.0f)
            m_ShiftCooldown -= dt;

        // Визуальный переход
        if (m_IsTransitioning) {
            m_TransitionTimer -= dt;
            if (m_TransitionTimer <= 0.0f)
                m_IsTransitioning = false;
        }

        // Заряд устройства
        if (m_CurrentTimeline == Timeline::Past) {
            m_DeviceCharge = std::max(0.0f, m_DeviceCharge - m_DrainRate * dt);
        } else {
            m_DeviceCharge = std::min(m_MaxCharge, m_DeviceCharge + m_RechargeRate * dt);
        }
    }

    // ═══════════════════════════════════════════════
    // Рендер визуальных эффектов перехода
    // ═══════════════════════════════════════════════
    void renderTransitionEffect(sf::RenderWindow& window) const {
        if (!m_IsTransitioning) return;

        float W = static_cast<float>(Config::SCREEN_WIDTH);
        float H = static_cast<float>(Config::SCREEN_HEIGHT);

        float progress = 1.0f - (m_TransitionTimer / m_TransitionDuration);

        // Мерцающие горизонтальные полосы (как помехи на старом ТВ)
        int stripeCount = 20 + static_cast<int>(progress * 40);
        for (int i = 0; i < stripeCount; ++i) {
            float y = static_cast<float>((i * 37 + static_cast<int>(progress * 1000)) % static_cast<int>(H));
            float height = 2.0f + std::sin(progress * 10.0f + i) * 3.0f;

            sf::RectangleShape stripe({W, std::max(1.0f, height)});
            stripe.setPosition(0, y);

            if (m_CurrentTimeline == Timeline::Past) {
                // Прошлое — голубоватые помехи
                stripe.setFillColor(sf::Color(100, 180, 255, static_cast<sf::Uint8>(80 * (1.0f - progress))));
            } else {
                // Настоящее — оранжевые помехи
                stripe.setFillColor(sf::Color(255, 150, 50, static_cast<sf::Uint8>(80 * (1.0f - progress))));
            }
            window.draw(stripe);
        }

        // Вспышка в момент переключения
        if (progress < 0.3f) {
            sf::RectangleShape flash({W, H});
            flash.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(200 * (1.0f - progress / 0.3f))));
            window.draw(flash);
        }
    }

    // ═══════════════════════════════════════════════
    // Рендер индикатора таймлайна и заряда на HUD
    // ═══════════════════════════════════════════════
    void renderHUD(sf::RenderWindow& window, const sf::Font* font) const {
        float W = static_cast<float>(Config::SCREEN_WIDTH);

        // Индикатор текущего таймлайна
        sf::RectangleShape indicator({12, 12});
        indicator.setPosition(W / 2 - 6, 15);

        if (m_CurrentTimeline == Timeline::Past) {
            indicator.setFillColor(sf::Color(100, 180, 255, 200));
        } else {
            indicator.setFillColor(sf::Color(255, 150, 50, 200));
        }
        window.draw(indicator);

        // Шкала заряда устройства
        float chargeW = 80.0f;
        float chargePct = m_DeviceCharge / m_MaxCharge;

        sf::RectangleShape chargeBg({chargeW, 6});
        chargeBg.setPosition(W / 2 - chargeW / 2, 30);
        chargeBg.setFillColor(sf::Color(30, 30, 30, 150));
        window.draw(chargeBg);

        sf::RectangleShape chargeFill({chargeW * chargePct, 6});
        chargeFill.setPosition(W / 2 - chargeW / 2, 30);
        sf::Color chargeColor = (m_DeviceCharge < 20.0f)
            ? sf::Color(255, 50, 50, 220)
            : sf::Color(100, 200, 255, 220);
        chargeFill.setFillColor(chargeColor);
        window.draw(chargeFill);

        // Текст
        if (font) {
            sf::Text label;
            label.setFont(*font);
            label.setCharacterSize(10);

            std::string tlName = (m_CurrentTimeline == Timeline::Past) ? "PAST" : "PRESENT";
            label.setString("[T] " + tlName);
            label.setFillColor(indicator.getFillColor());
            label.setPosition(W / 2 - 25, 38);
            window.draw(label);

            if (m_ShiftCooldown > 0.0f) {
                sf::Text cd;
                cd.setFont(*font);
                cd.setCharacterSize(9);
                cd.setString("CD:" + std::to_string(static_cast<int>(m_ShiftCooldown * 10) / 10) + "s");
                cd.setFillColor(sf::Color(150, 150, 150));
                cd.setPosition(W / 2 - 15, 50);
                window.draw(cd);
            }
        }
    }

    // ═══════════════════════════════════════════════
    // Геттеры
    // ═══════════════════════════════════════════════
    Timeline getCurrentTimeline()  const { return m_CurrentTimeline; }
    bool     isPast()              const { return m_CurrentTimeline == Timeline::Past; }
    bool     isPresent()           const { return m_CurrentTimeline == Timeline::Present; }
    bool     isTransitioning()     const { return m_IsTransitioning; }
    float    getDeviceCharge()     const { return m_DeviceCharge; }
    float    getMaxCharge()        const { return m_MaxCharge; }
    float    getChargePercent()    const { return m_DeviceCharge / m_MaxCharge; }
    float    getCooldown()         const { return m_ShiftCooldown; }
    bool     isReady()             const { return m_ShiftCooldown <= 0.0f && m_DeviceCharge >= 5.0f; }
    bool     isInitialized()       const { return m_Initialized; }

private:
    // ── Сохранение настоящего из GameState ──
    void savePresentFromGameState(const GameState& gs) {
        m_PresentSnapshot.sectorMap      = gs.sectorMap;
        m_PresentSnapshot.wallDurability  = gs.wallDurability;
        m_PresentSnapshot.erosionMap      = gs.etherErosionMap;
        m_PresentSnapshot.enemies         = gs.enemies;
        m_PresentSnapshot.lootContainers  = gs.lootContainers;
    }

    // ── Сохранение прошлого из GameState ──
    void savePastFromGameState(const GameState& gs) {
        m_PastSnapshot.sectorMap      = gs.sectorMap;
        m_PastSnapshot.wallDurability  = gs.wallDurability;
        m_PastSnapshot.erosionMap      = gs.etherErosionMap;
        m_PastSnapshot.enemies         = gs.enemies;
        m_PastSnapshot.lootContainers  = gs.lootContainers;
    }

    // ── Загрузка настоящего в GameState ──
    void loadPresentToGameState(GameState& gs) {
        gs.sectorMap      = m_PresentSnapshot.sectorMap;
        gs.wallDurability  = m_PresentSnapshot.wallDurability;
        gs.etherErosionMap = m_PresentSnapshot.erosionMap;
        gs.enemies         = m_PresentSnapshot.enemies;
        gs.lootContainers  = m_PresentSnapshot.lootContainers;
    }

    // ── Загрузка прошлого в GameState ──
    void loadPastToGameState(GameState& gs) {
        gs.sectorMap      = m_PastSnapshot.sectorMap;
        gs.wallDurability  = m_PastSnapshot.wallDurability;
        gs.etherErosionMap = m_PastSnapshot.erosionMap;
        gs.enemies         = m_PastSnapshot.enemies;
        gs.lootContainers  = m_PastSnapshot.lootContainers;
    }

    // ═══════════════════════════════════════════════
    // Генерация прошлого (другая карта, другие враги)
    // Убежище 17 ДО катастрофы:
    //   - Все стены целые, больше комнат
    //   - Нет эрозии
    //   - Роботы-охранники вместо Роя
    // ═══════════════════════════════════════════════
    void generatePastTimeline(GameState& gs, EnemySpawner& spawner) {
        // Зануляем
        for (int x = 0; x < Config::MAP_WIDTH; ++x) {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y) {
                m_PastSnapshot.sectorMap[x][y]     = 0;
                m_PastSnapshot.wallDurability[x][y] = 0;
                m_PastSnapshot.erosionMap[x][y]     = 0.0f;  // Нет эрозии в прошлом!
            }
        }

        // ── Более сложная планировка Убежища (до разрушения) ──
        // Внешние стены (3..16)
        for (int x = 3; x < 17; ++x) {
            for (int y = 3; y < 17; ++y) {
                if (x == 3 || x == 16 || y == 3 || y == 16) {
                    m_PastSnapshot.sectorMap[x][y] = 1;
                    m_PastSnapshot.wallDurability[x][y] = 200;
                }
            }
        }

        // Внутренние перегородки (комнаты)
        for (int y = 3; y < 17; ++y) {
            m_PastSnapshot.sectorMap[8][y] = 1;
            m_PastSnapshot.wallDurability[8][y] = 150;
            m_PastSnapshot.sectorMap[12][y] = 1;
            m_PastSnapshot.wallDurability[12][y] = 150;
        }
        for (int x = 3; x < 17; ++x) {
            m_PastSnapshot.sectorMap[x][8] = 1;
            m_PastSnapshot.wallDurability[x][8] = 150;
            m_PastSnapshot.sectorMap[x][12] = 1;
            m_PastSnapshot.wallDurability[x][12] = 150;
        }

        // Двери (проходы между комнатами)
        int doors[][2] = {
            {8, 5}, {8, 10}, {8, 14},
            {12, 5}, {12, 10}, {12, 14},
            {5, 8}, {10, 8}, {14, 8},
            {5, 12}, {10, 12}, {14, 12},
            {9, 3}, {9, 16}   // Главные шлюзы
        };
        for (auto& d : doors) {
            m_PastSnapshot.sectorMap[d[0]][d[1]] = 0;
        }

        // ── Роботы-охранники вместо Роя ──
        m_PastSnapshot.enemies.clear();

        // Спавним патрульных роботов
        Vector3D robotPositions[] = {
            {5.0f, 5.0f, 0.0f}, {10.0f, 5.0f, 0.0f}, {14.0f, 5.0f, 0.0f},
            {5.0f, 10.0f, 0.0f}, {14.0f, 10.0f, 0.0f},
            {5.0f, 14.0f, 0.0f}, {10.0f, 14.0f, 0.0f}, {14.0f, 14.0f, 0.0f}
        };

        for (const auto& pos : robotPositions) {
            Enemy robot;
            robot.position = pos;
            robot.health   = 80.0f;   // Роботы покрепче верминов
            robot.speed    = 1.8f;    // Но медленнее
            robot.radius   = 0.4f;
            robot.isAlive  = true;
            m_PastSnapshot.enemies.push_back(robot);
        }

        // ── Лут прошлого (более ценный, неоткрытый) ──
        m_PastSnapshot.lootContainers.clear();

        LootContainer pastCrate1;
        pastCrate1.position = {6.0f, 6.0f, 0.0f};
        pastCrate1.type = LootContainerType::IronSafe;
        pastCrate1.isOpened = false;
        pastCrate1.containsItems.push_back({401, ItemType::Resource, 10, 0.5f, "PRE-WAR REPAIR KIT"});
        m_PastSnapshot.lootContainers.push_back(pastCrate1);

        LootContainer pastCrate2;
        pastCrate2.position = {13.0f, 13.0f, 0.0f};
        pastCrate2.type = LootContainerType::IronSafe;
        pastCrate2.isOpened = false;
        pastCrate2.containsItems.push_back({102, ItemType::Weapon, 1, 2.0f, "PROTOTYPE XO-16"});
        m_PastSnapshot.lootContainers.push_back(pastCrate2);

        std::cout << "[TIMESHIFT] Прошлое сгенерировано: "
                  << m_PastSnapshot.enemies.size() << " роботов, "
                  << m_PastSnapshot.lootContainers.size() << " контейнеров." << std::endl;
    }
};

}  // namespace bunker
