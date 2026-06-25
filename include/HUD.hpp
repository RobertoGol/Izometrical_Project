#pragma once

#include <SFML/Graphics.hpp>
#include "Types.hpp"
#include "GameState.hpp"
#include "PlayerController.hpp"
#include "Tactics.hpp"
#include "TitanAI.hpp"
#include "VehicleManager.hpp"
#include "Inventory.hpp"
#include "Constants.hpp"
#include <string>
#include <cmath>
#include <algorithm>

namespace bunker {

class HUD {
private:
    sf::Font m_Font;
    bool     m_FontLoaded = false;

    // ── Шкала ──
    void drawBar(sf::RenderWindow& window, float x, float y, float width, float height,
                 float percent, sf::Color bgColor, sf::Color fillColor,
                 sf::Color borderColor = sf::Color::Transparent) {
        percent = std::clamp(percent, 0.0f, 1.0f);

        sf::RectangleShape bg({width, height});
        bg.setPosition(x, y);
        bg.setFillColor(bgColor);
        if (borderColor != sf::Color::Transparent) {
            bg.setOutlineThickness(1.0f);
            bg.setOutlineColor(borderColor);
        }
        window.draw(bg);

        sf::RectangleShape fill({width * percent, height});
        fill.setPosition(x, y);
        fill.setFillColor(fillColor);
        window.draw(fill);
    }

    // ── Текст ──
    void drawText(sf::RenderWindow& window, const std::string& str,
                  float x, float y, int size, sf::Color color = sf::Color::White) {
        if (!m_FontLoaded) return;
        sf::Text text;
        text.setFont(m_Font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(x, y);
        window.draw(text);
    }

public:
    HUD() = default;

    void loadFont(const std::string& fontPath = "assets/fonts/default.ttf") {
        if (m_Font.loadFromFile(fontPath)) {
            m_FontLoaded = true;
        }
    }

    // ═══════════════════════════════════════════════
    // Основной рендер HUD
    // ═══════════════════════════════════════════════
    void render(sf::RenderWindow& window, const GameState& gs,
                const PlayerController& playerCtrl,
                const TacticsManager& tactics,
                const TitanAI& titanAI,
                const VehicleManager& vehicles,
                const PlayerInventory& inventory) {

        float W = static_cast<float>(Config::SCREEN_WIDTH);
        float H = static_cast<float>(Config::SCREEN_HEIGHT);

        if (vehicles.isPlayerInVehicle()) {
            renderVehicleHUD(window, gs, vehicles, W, H);
        } else if (gs.playerMode == UnitMode::Titan) {
            // Ретро-рамка Pip-Pad ТОЛЬКО для Танка БТ-7274
            drawTankFrame(window, W, H);
            renderTankHUD(window, gs, titanAI, W, H);
        } else {
            renderScoutHUD(window, gs, playerCtrl, tactics, inventory, W, H);
        }

        // ── Общее ──
        renderScore(window, gs, W);
        renderErosionWarning(window, gs, W, H);
        renderPipPadHint(window, gs, W, H);
    }

private:
    // ── Ретро-рамка (только для Танка) ──
    void drawTankFrame(sf::RenderWindow& window, float W, float H) {
        sf::Color frameColor(230, 115, 25, 80);
        drawBar(window, 10, 10, W - 20, 3, 1.0f, frameColor, frameColor);
        drawBar(window, 10, H - 13, W - 20, 3, 1.0f, frameColor, frameColor);
        drawBar(window, 10, 10, 3, H - 20, 1.0f, frameColor, frameColor);
        drawBar(window, W - 13, 10, 3, H - 20, 1.0f, frameColor, frameColor);
    }

    // ═══════════════════════════════════════════════
    // HUD Пехотинца (Scout) — Fallout 76 + Log Horizon
    // ═══════════════════════════════════════════════
    void renderScoutHUD(sf::RenderWindow& window, const GameState& gs,
                        const PlayerController& ctrl, const TacticsManager& tactics,
                        const PlayerInventory& inventory,
                        float W, float H) {

        float barX = 40.0f;
        float barBaseY = H - 110.0f;

        // ── XP (над здоровьем) — тонкая жёлтая полоска ──
        float xpRequired = static_cast<float>(Config::BASE_XP_PER_LEVEL +
                           (gs.characterProg.level - 1) * Config::XP_INCREMENT);
        float xpPct = (xpRequired > 0.0f) ? static_cast<float>(gs.characterProg.experience) / xpRequired : 0.0f;
        drawBar(window, barX, barBaseY, 260, 5, xpPct,
                sf::Color(40, 40, 40, 130), sf::Color(255, 220, 50, 200));
        std::string lvlStr = "LV " + std::to_string(gs.characterProg.level);
        drawText(window, lvlStr, barX + 265, barBaseY - 2, 10, sf::Color(255, 220, 50));

        // ── HP (здоровье) ──
        float hpPct = gs.playerHealth / gs.playerMaxHealth;
        drawBar(window, barX, barBaseY + 10, 250, 14, hpPct,
                sf::Color(50, 50, 50, 130), sf::Color(255, 50, 50, 220),
                sf::Color(80, 30, 30, 150));
        std::string hpStr = std::to_string(static_cast<int>(gs.playerHealth)) + "/"
                          + std::to_string(static_cast<int>(gs.playerMaxHealth));
        drawText(window, hpStr, barX + 5, barBaseY + 10, 11, sf::Color(255, 150, 150));

        // ── AP (Action Points / стамина — Log Horizon стиль) ──
        float apPct = ctrl.getStaminaPercent();
        drawBar(window, barX, barBaseY + 30, 200, 8, apPct,
                sf::Color(50, 50, 50, 130), sf::Color(100, 220, 50, 200),
                sf::Color(40, 80, 20, 150));
        drawText(window, "AP", barX + 205, barBaseY + 27, 10, sf::Color(100, 220, 50));

        // ── Эрозия (фиолетовая, под AP) ──
        float erPct = gs.playerErosionLevel / 100.0f;
        if (erPct > 0.01f) {
            drawBar(window, barX, barBaseY + 44, 150, 5, erPct,
                    sf::Color(30, 30, 30, 150), sf::Color(150, 50, 200, 180));
            drawText(window, "ETHER", barX + 155, barBaseY + 41, 9, sf::Color(150, 50, 200));
        }

        // ── Класс пилота + тактика ──
        std::string classInfo = tactics.getActiveClassName();
        if (tactics.isTacticalActive) {
            classInfo += " [ACTIVE]";
            drawText(window, classInfo, barX, barBaseY + 55, 11, sf::Color(50, 255, 150));
        } else if (tactics.tacticalCooldown > 0.0f) {
            classInfo += " [" + std::to_string(static_cast<int>(tactics.tacticalCooldown)) + "s]";
            drawText(window, classInfo, barX, barBaseY + 55, 11, sf::Color(120, 120, 120));
        } else {
            drawText(window, classInfo, barX, barBaseY + 55, 11, sf::Color(50, 255, 50));
        }

        // ── Нырок ──
        if (ctrl.isDiving()) {
            drawText(window, ">> DIVE <<", W / 2 - 40, H - 140, 14, sf::Color(255, 255, 100));
        }

        // ── Вес рюкзака ──
        std::string wt = std::to_string(static_cast<int>(inventory.getCurrentWeight()))
                       + "/" + std::to_string(static_cast<int>(inventory.getMaxWeight())) + " kg";
        drawText(window, wt, W - 120, H - 30, 11, sf::Color(180, 180, 180));

        // ── Подсказка Pip-Pad найден ──
        if (gs.bunkerProgression.hasFoundPipPad) {
            drawText(window, "[Q] Ability  [1-7] Class", barX, barBaseY + 70, 10, sf::Color(100, 100, 100));
        }
    }

    // ═══════════════════════════════════════════════
    // HUD Танка (БТ-7274) — тяжёлый аналоговый интерфейс
    // ═══════════════════════════════════════════════
    void renderTankHUD(sf::RenderWindow& window, const GameState& gs,
                       const TitanAI& titanAI, float W, float H) {
        sf::Color tankOrange(230, 115, 25);
        sf::Color darkBg(25, 50, 25, 100);

        float barX = 50.0f;

        // ── Hull HP ──
        float hpPct = gs.titan.health / gs.titan.maxHealth;
        drawBar(window, barX, H - 100, 300, 18, hpPct,
                darkBg, sf::Color(255, 80, 30, 220), sf::Color(120, 50, 10, 150));
        drawText(window, "BT-7274 HULL", barX + 2, H - 102, 12, tankOrange);

        // ── Vortex Shield ──
        float vortexPct = titanAI.getVortexEnergy() / 100.0f;
        drawBar(window, barX, H - 75, 300, 12, vortexPct,
                darkBg, sf::Color(80, 150, 255, 220));
        std::string vLabel = titanAI.isVortexActive() ? "VORTEX [ACTIVE]" : "VORTEX SHIELD";
        drawText(window, vLabel, barX + 2, H - 77, 10, sf::Color(80, 150, 255));

        // ── Tracks ──
        float tracksPct = gs.titan.systems.tracksCondition / 100.0f;
        drawBar(window, barX, H - 58, 300, 10, tracksPct,
                sf::Color(50, 25, 0, 100), tankOrange);
        std::string trkLabel = (tracksPct < 0.4f) ? "TRACKS [DAMAGED]" : "TRACKS";
        drawText(window, trkLabel, barX + 2, H - 60, 9,
                 (tracksPct < 0.4f) ? sf::Color::Red : tankOrange);

        // ── Turret ──
        float turretPct = gs.titan.systems.turretStatus / 100.0f;
        drawBar(window, barX, H - 44, 300, 10, turretPct,
                sf::Color(50, 25, 0, 100), tankOrange);
        std::string turLabel = (turretPct < 0.5f) ? "TURRET [MALFUNCTION]" : "TURRET";
        drawText(window, turLabel, barX + 2, H - 46, 9,
                 (turretPct < 0.5f) ? sf::Color::Red : tankOrange);

        // ── Core Charge ──
        float corePct = titanAI.getCoreCharge() / 100.0f;
        drawBar(window, barX, H - 28, 300, 22, corePct,
                sf::Color(50, 50, 50, 130), sf::Color(255, 180, 0, 230),
                sf::Color(100, 80, 0, 150));
        std::string coreLabel = titanAI.isCoreActive() ? "!! CORE OVERDRIVE !!" : "CORE CHARGE";
        sf::Color coreColor = titanAI.isCoreActive() ? sf::Color(255, 50, 50) : sf::Color(255, 200, 50);
        drawText(window, coreLabel, barX + 2, H - 30, 13, coreColor);

        // ── Weapon loadout ──
        std::string wpn;
        switch (titanAI.getLoadout()) {
            case AncientLoadout::XO16_SolidKinetic:      wpn = "[1] XO-16 AUTOCANNON"; break;
            case AncientLoadout::Scorch_ThermiteMortar:   wpn = "[2] SCORCH MORTAR"; break;
            case AncientLoadout::Ion_SplitLaser_Vacuum:   wpn = "[3] ION LASER"; break;
        }
        drawText(window, wpn, W - 280, H - 30, 12, tankOrange);

        // ── Подсказки ──
        drawText(window, "[Q] Vortex  [Tab] Dismount", barX, H - 5, 10, sf::Color(100, 100, 100));
    }

    // ═══════════════════════════════════════════════
    // HUD Транспорта
    // ═══════════════════════════════════════════════
    void renderVehicleHUD(sf::RenderWindow& window, const GameState& gs,
                          const VehicleManager& vehicles, float W, float H) {
        sf::Color amber(255, 180, 0);
        const auto* cfg = vehicles.getActiveConfig();
        std::string name = cfg ? cfg->displayName : "VEHICLE";

        drawText(window, name, W / 2 - 80, H - 90, 14, amber);

        // Спидометр
        drawBar(window, W / 2 - 200, H - 65, 400, 18, 0.5f,
                sf::Color(40, 25, 0, 130), amber, sf::Color(100, 80, 0, 100));
        drawText(window, "SPEED", W / 2 - 198, H - 67, 10, amber);

        if (cfg && cfg->driveType == "pressure") {
            float pressurePct = vehicles.getCarPressurePercent();
            drawBar(window, W / 2 - 200, H - 40, 200, 8, pressurePct,
                    sf::Color(50, 25, 0, 130), sf::Color(200, 100, 0, 200));
            drawText(window, "STEAM PRESSURE", W / 2 - 198, H - 42, 9, sf::Color(200, 100, 0));
        }

        drawText(window, "[X] Dismount", W / 2 - 45, H - 20, 12, sf::Color(200, 200, 200));
    }

    // ═══════════════════════════════════════════════
    // Общие элементы
    // ═══════════════════════════════════════════════
    void renderScore(sf::RenderWindow& window, const GameState& gs, float W) {
        drawText(window, "SCORE: " + std::to_string(gs.score), W - 180, 20, 14, sf::Color(50, 255, 50));
    }

    void renderErosionWarning(sf::RenderWindow& window, const GameState& gs, float W, float H) {
        if (gs.playerErosionLevel > 70.0f) {
            sf::RectangleShape warn({20, 20});
            warn.setPosition(W - 60, 50);
            warn.setFillColor(sf::Color(255, 50, 50, 150));
            window.draw(warn);
            drawText(window, "!", W - 55, 48, 16, sf::Color::Red);
        }
    }

    void renderPipPadHint(sf::RenderWindow& window, const GameState& gs, float W, float H) {
        if (!gs.bunkerProgression.hasFoundPipPad) {
            drawText(window, "Find the Pip-Pad to unlock abilities!",
                     W / 2 - 150, 30, 13, sf::Color(200, 200, 50));
        }
    }
};

}  // namespace bunker
