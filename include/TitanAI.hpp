#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Collisions.hpp"
#include "Constants.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace bunker {

// ═══════════════════════════════════════════════════════
// Типы аналогового вооружения БТ-7274
// ═══════════════════════════════════════════════════════
enum class AncientLoadout {
    XO16_SolidKinetic,       // Автопушка XO-16 (быстрый огонь, средний урон)
    Scorch_ThermiteMortar,   // Миномёт Скорча (медленный, тяжёлый, нет Вортекса)
    Ion_SplitLaser_Vacuum    // Ион-лазер (быстрый, точный, без отдачи)
};

// ═══════════════════════════════════════════════════════
// Баллистический профиль кулачковых валов
// Определяет поведение орудия при стрельбе
// ═══════════════════════════════════════════════════════
struct BallisticCamProfile {
    float hydraulicPressureBar = 350.0f;   // Давление гидросистемы (компенсация отдачи)
    float recoilDecayMs        = 120.0f;   // Затухание колебаний рессор
    float mechanicalLeadFactor = 1.02f;    // Угол упреждения шестерёнчатого привода
};

// ═══════════════════════════════════════════════════════
// Органы управления из кабины Авангарда
// ═══════════════════════════════════════════════════════
struct PilotInputControls {
    float throttleLever      = 0.0f;   // Рычаг газа (0.0 — 1.0)
    float steeringWheelAngle = 0.0f;   // Угол штурвала наведения
    bool  triggerPressed     = false;   // Механический спуск
    float handTremorHz       = 0.0f;   // Дрожание рук от стресса
};

// ═══════════════════════════════════════════════════════
// Физическое состояние грунта под ногами
// ═══════════════════════════════════════════════════════
struct TerrainFrictionData {
    float surfaceSlickness     = 0.0f;  // 0.0 = сухой бетон, 1.0 = лёд/грязь
    float structuralIntegrity  = 1.0f;  // Прочность почвы
};

// ═══════════════════════════════════════════════════════
// Полная симуляция Танка БТ-7274 (Vanguard-class Titan)
//
// Из Video_Game_Izom/Data/Physics/Tank.hpp + Tank.cpp:
//   - Два режима: Автономный (ИИ) / Симбиотический (Пилот внутри)
//   - Вортекс-щит (Q): ловит пули → выбрасывает обратно
//   - Смена оружия (1-2-3): XO-16 / Scorch / Ion с задержкой 0.4с
//   - Core Overdrive: накопление заряда → удвоение огня
//   - Combat Anchor: автономная навигация к 40% между Пилотом и Роем
//   - Anti-Blocking: уступает дорогу Пилоту в радиусе 6м
//   - Гальванический мост: фильтрация дрожания рук, стабилизация на грязи
//   - Котёл-реактор: давление пара, температура, перегруз
//   - Посадка/высадка по Tab с проверкой расстояния
// ═══════════════════════════════════════════════════════

class TitanAI {
private:
    // ── Физика шасси ──
    const float m_TitanAcceleration = 4.0f;
    const float m_TitanDeceleration = 6.0f;
    const float m_ExclusionRadiusSq = 36.0f;  // Anti-Blocking: 6 метров

    Vector3D m_Velocity = {0.0f, 0.0f, 0.0f};

    // ── Вооружение ──
    AncientLoadout     m_ActiveLoadout = AncientLoadout::XO16_SolidKinetic;
    BallisticCamProfile m_CurrentCamProfile;

    // ── Вортекс-щит ──
    bool  m_VortexActive       = false;
    float m_VortexEnergy       = 100.0f;
    int   m_CaughtBulletsCount = 0;

    // ── Котёл-реактор (Аналоговое Ядро) ──
    float m_CoreChargePercent      = 0.0f;
    float m_BoilerSteamPressure    = 350.0f;   // Давление пара в барах
    float m_CoolantTemperature     = 180.0f;   // Температура охладителя (°C)
    bool  m_CoreOverdriveActive    = false;

    // ── Механическая смена оружия ──
    float m_ReMapTimer   = 0.0f;
    bool  m_IsReMapping  = false;

    // ── Стресс пилота (влияет на дрожание рук) ──
    float m_PilotStress = 0.0f;

public:
    TitanAI() {
        m_CurrentCamProfile = {350.0f, 120.0f, 1.02f};
    }

    // ═══════════════════════════════════════════════
    // Посадка в Танк (Tab при приближении)
    // ═══════════════════════════════════════════════
    bool tryMount(GameState& gs) {
        if (gs.playerMode == UnitMode::Titan) return false;  // Уже внутри

        float dx = gs.playerPos.x - gs.titan.position.x;
        float dy = gs.playerPos.y - gs.titan.position.y;
        float distSq = dx * dx + dy * dy;

        if (distSq > 4.0f) {
            std::cout << "[BT-7274] Слишком далеко для посадки." << std::endl;
            return false;
        }

        gs.playerMode = UnitMode::Titan;
        gs.titan.isPiloted = true;
        gs.playerPos = gs.titan.position;

        std::cout << "[BT-7274] Протокол 3: Защитить Пилота. Связь установлена." << std::endl;
        return true;
    }

    // ═══════════════════════════════════════════════
    // Высадка из Танка (Tab)
    // ═══════════════════════════════════════════════
    void dismount(GameState& gs) {
        if (gs.playerMode != UnitMode::Titan) return;

        gs.playerMode = UnitMode::Scout;
        gs.titan.isPiloted = false;

        // Высаживаем рядом чтобы не застрять в корпусе
        gs.playerPos.x = gs.titan.position.x + 1.0f;
        gs.playerPos.y = gs.titan.position.y;

        // Проверяем не попали ли в стену — если да, пробуем другие стороны
        if (Collisions::checkWorldCollision(gs, gs.playerPos.x, gs.playerPos.y, Config::PLAYER_RADIUS)) {
            gs.playerPos.x = gs.titan.position.x - 1.0f;
            gs.playerPos.y = gs.titan.position.y;
        }
        if (Collisions::checkWorldCollision(gs, gs.playerPos.x, gs.playerPos.y, Config::PLAYER_RADIUS)) {
            gs.playerPos.x = gs.titan.position.x;
            gs.playerPos.y = gs.titan.position.y + 1.0f;
        }
        if (Collisions::checkWorldCollision(gs, gs.playerPos.x, gs.playerPos.y, Config::PLAYER_RADIUS)) {
            gs.playerPos.x = gs.titan.position.x;
            gs.playerPos.y = gs.titan.position.y - 1.0f;
        }

        m_VortexActive = false;
        m_CaughtBulletsCount = 0;

        std::cout << "[BT-7274] Высадка. Переход в автономный режим." << std::endl;
    }

    // ═══════════════════════════════════════════════
    // Главный тик ИИ Танка каждый кадр
    // ═══════════════════════════════════════════════
    void update(GameState& gs, const InputSnapshot& input, float dt) {

        // ── Котёл-реактор: давление и температура ──
        updateBoiler(dt);

        // ── Накопление заряда Core ──
        if (!m_CoreOverdriveActive) {
            m_CoreChargePercent = std::min(100.0f, m_CoreChargePercent + 4.5f * dt);
        } else {
            m_CoreChargePercent = std::max(0.0f, m_CoreChargePercent - 18.0f * dt);
            if (m_CoreChargePercent <= 0.0f) {
                m_CoreOverdriveActive = false;
                m_BoilerSteamPressure = 350.0f;  // Сброс давления после перегруза
                m_CoolantTemperature = 180.0f;
            }
        }

        // ── Кулдаун стрельбы Танка ──
        if (gs.titan.fireCooldown > 0.0f)
            gs.titan.fireCooldown -= dt;

        // ── Механическая смена оружия (задержка 0.4с) ──
        if (m_IsReMapping) {
            m_ReMapTimer -= dt;
            if (m_ReMapTimer <= 0.0f) {
                m_IsReMapping = false;
                std::cout << "[BT-7274] Калибр сменён. Орудие готово." << std::endl;
            }
            return;  // Манипуляторы заблокированы
        }

        // ── Горячая смена оружия (1-2-3, только в режиме Танка) ──
        if (gs.playerMode == UnitMode::Titan) {
            if (input.selectClass[0]) triggerReMap(gs, AncientLoadout::XO16_SolidKinetic);
            if (input.selectClass[1]) triggerReMap(gs, AncientLoadout::Scorch_ThermiteMortar);
            if (input.selectClass[2]) triggerReMap(gs, AncientLoadout::Ion_SplitLaser_Vacuum);

            // Вортекс-щит (Q удержание)
            bool holdingQ = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
            updateVortexShield(gs, holdingQ, dt);

            // Core Overdrive (F)
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
                if (validateCoreOverdriveTrigger(gs)) {
                    executeCoreOverdrive();
                }
            }
        }

        // ── Стресс пилота ──
        updatePilotStress(gs, dt);

        // ── Автономный / Симбиотический ──
        if (!gs.titan.isPiloted) {
            updateAutonomousMode(gs, dt);
        } else {
            updatePilotedMode(gs, input, dt);
        }
    }

    // ═══════════════════════════════════════════════
    // Стрельба из кабины (вызывается из main.cpp)
    // Учитывает loadout, дрожание рук, Core Overdrive
    // ═══════════════════════════════════════════════
    void fireFromCockpit(GameState& gs) {
        if (gs.titan.fireCooldown > 0.0f) return;
        if (m_VortexActive) return;
        if (m_IsReMapping) return;

        float mdx = gs.mouseWorldPos.x - gs.titan.position.x;
        float mdy = gs.mouseWorldPos.y - gs.titan.position.y;
        float mLenSq = mdx * mdx + mdy * mdy;
        if (mLenSq < 0.01f) return;

        float mLen = std::sqrt(mLenSq);
        Vector3D targetDir = {mdx / mLen, mdy / mLen, 0.0f};

        // Дрожание рук при высоком стрессе
        if (m_PilotStress > 30.0f) {
            float tremor = (m_PilotStress / 100.0f) * 0.15f;
            float spread = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * tremor;
            targetDir.x += spread;
            targetDir.y += spread;
        }

        // Дрожание от повреждённой турели (<50%)
        if (gs.titan.systems.turretStatus < 50.0f) {
            float broken = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 0.25f;
            targetDir.x += broken;
            targetDir.y += broken;
        }

        Bullet b;
        b.start   = gs.titan.position;
        b.current = gs.titan.position;

        switch (m_ActiveLoadout) {
        case AncientLoadout::XO16_SolidKinetic:
            b.type  = BulletType::Standard;
            b.speed = 36.0f;
            gs.bullets.push_back(b);
            b.direction = targetDir;
            gs.bullets.back().direction = targetDir;
            gs.titan.fireCooldown = m_CoreOverdriveActive ? 0.03f : 0.08f;
            break;

        case AncientLoadout::Scorch_ThermiteMortar:
            b.type  = BulletType::Standard;
            b.speed = 18.0f;
            b.direction = targetDir;
            gs.bullets.push_back(b);
            gs.titan.fireCooldown = 0.65f;
            break;

        case AncientLoadout::Ion_SplitLaser_Vacuum:
            b.type  = BulletType::Standard;
            b.speed = 45.0f;
            b.direction = targetDir;
            gs.bullets.push_back(b);
            gs.titan.fireCooldown = 0.14f;
            break;
        }
    }

    // ═══════════════════════════════════════════════
    // Автономная стрельба (когда Пилот снаружи)
    // БТ-7274 сам ищет цели в радиусе 20 клеток
    // ═══════════════════════════════════════════════
    void autoFire(GameState& gs) {
        if (gs.titan.isPiloted) return;
        if (gs.titan.fireCooldown > 0.0f) return;
        if (m_VortexActive || m_IsReMapping) return;
        if (gs.enemies.empty()) return;

        for (auto& e : gs.enemies) {
            if (!e.isAlive) continue;

            float edx = e.position.x - gs.titan.position.x;
            float edy = e.position.y - gs.titan.position.y;
            float eDistSq = edx * edx + edy * edy;

            if (eDistSq <= 400.0f) {  // 20 клеток
                float eDist = std::sqrt(eDistSq);
                Vector3D targetDir = {edx / eDist, edy / eDist, 0.0f};

                Bullet b;
                b.start     = gs.titan.position;
                b.current   = gs.titan.position;
                b.direction = targetDir;
                b.type      = BulletType::Standard;
                b.speed     = 36.0f;
                gs.bullets.push_back(b);

                gs.titan.fireCooldown = 0.09f;
                break;
            }
        }
    }

    // ═══════════════════════════════════════════════
    // Гальванический мост: фильтрация и стабилизация ввода
    // ═══════════════════════════════════════════════
    PilotInputControls filterAndStabilizeInputs(
        const PilotInputControls& rawInput,
        const TerrainFrictionData& terrain) {

        PilotInputControls stabilized = rawInput;

        // Магнитные демпферы: гасят дрожание рук при стрессе > 5 Гц
        if (rawInput.handTremorHz > 5.0f) {
            stabilized.steeringWheelAngle = rawInput.steeringWheelAngle * 0.85f;
        }

        // Автобаланс на скользком грунте
        if (terrain.surfaceSlickness > 0.6f) {
            float balanceOffset = std::sin(rawInput.throttleLever)
                                * terrain.surfaceSlickness * 15.0f;
            // Подмешивается в анимацию шасси (визуальный эффект)
            (void)balanceOffset;
        }

        return stabilized;
    }

    // ═══════════════════════════════════════════════
    // Геттеры
    // ═══════════════════════════════════════════════
    bool  isVortexActive()       const { return m_VortexActive; }
    float getVortexEnergy()      const { return m_VortexEnergy; }
    bool  isCoreActive()         const { return m_CoreOverdriveActive; }
    float getCoreCharge()        const { return m_CoreChargePercent; }
    AncientLoadout getLoadout()  const { return m_ActiveLoadout; }
    bool  isReMapping()          const { return m_IsReMapping; }
    float getBoilerPressure()    const { return m_BoilerSteamPressure; }
    float getCoolantTemp()       const { return m_CoolantTemperature; }
    float getPilotStress()       const { return m_PilotStress; }

    std::string getLoadoutName() const {
        switch (m_ActiveLoadout) {
            case AncientLoadout::XO16_SolidKinetic:    return "[1] XO-16 AUTOCANNON";
            case AncientLoadout::Scorch_ThermiteMortar: return "[2] SCORCH MORTAR";
            case AncientLoadout::Ion_SplitLaser_Vacuum: return "[3] ION LASER";
            default: return "UNKNOWN";
        }
    }

private:
    // ═══════════════════════════════════════════════
    // Котёл-реактор: давление пара и температура
    // ═══════════════════════════════════════════════
    void updateBoiler(float dt) {
        if (m_CoreOverdriveActive) {
            // Перегруз — давление и температура на максимуме
            m_BoilerSteamPressure = std::min(1200.0f, m_BoilerSteamPressure + 50.0f * dt);
            m_CoolantTemperature = std::min(980.0f, m_CoolantTemperature + 30.0f * dt);
        } else {
            // Нормальный режим — медленное стремление к норме
            m_BoilerSteamPressure += (350.0f - m_BoilerSteamPressure) * 0.5f * dt;
            m_CoolantTemperature += (180.0f - m_CoolantTemperature) * 0.3f * dt;
        }
    }

    // ═══════════════════════════════════════════════
    // Стресс пилота (влияет на дрожание рук)
    // Растёт от урона, падает со временем
    // ═══════════════════════════════════════════════
    void updatePilotStress(const GameState& gs, float dt) {
        // Стресс растёт если здоровье Танка падает
        float hpPercent = gs.titan.health / gs.titan.maxHealth;
        float targetStress = (1.0f - hpPercent) * 60.0f;

        // Стресс от повреждённых систем
        if (gs.titan.systems.turretStatus < 50.0f) targetStress += 15.0f;
        if (gs.titan.systems.tracksCondition < 40.0f) targetStress += 20.0f;
        if (gs.titan.systems.sensorLink < 30.0f) targetStress += 10.0f;

        // Плавное приближение к целевому стрессу
        m_PilotStress += (targetStress - m_PilotStress) * 2.0f * dt;
        m_PilotStress = std::clamp(m_PilotStress, 0.0f, 100.0f);
    }

    // ═══════════════════════════════════════════════
    // Смена оружия с механической задержкой 0.4с
    // ═══════════════════════════════════════════════
    void triggerReMap(GameState& gs, AncientLoadout newWeapon) {
        if (m_VortexActive || m_IsReMapping) return;
        if (m_ActiveLoadout == newWeapon) return;  // Уже этот калибр

        m_ActiveLoadout = newWeapon;
        m_IsReMapping = true;
        m_ReMapTimer = 0.4f;

        switch (newWeapon) {
        case AncientLoadout::XO16_SolidKinetic:
            m_CurrentCamProfile = {350.0f, 120.0f, 1.02f};
            gs.titan.currentWeapon = TankWeaponMode::AutoCannon;
            break;
        case AncientLoadout::Scorch_ThermiteMortar:
            m_CurrentCamProfile = {600.0f, 450.0f, 1.85f};
            gs.titan.currentWeapon = TankWeaponMode::Cannon;
            break;
        case AncientLoadout::Ion_SplitLaser_Vacuum:
            m_CurrentCamProfile = {100.0f, 10.0f, 1.0f};
            gs.titan.currentWeapon = TankWeaponMode::AutoCannon;
            break;
        }

        std::cout << "[BT-7274] Смена калибра → " << getLoadoutName() << std::endl;
    }

    // ═══════════════════════════════════════════════
    // Вортекс-щит: ловит пули → выплёвывает веером
    // У Скорча щита нет (по лору)
    // ═══════════════════════════════════════════════
    void updateVortexShield(GameState& gs, bool isHoldingQ, float dt) {
        if (m_ActiveLoadout == AncientLoadout::Scorch_ThermiteMortar) {
            m_VortexActive = false;
            return;
        }

        if (isHoldingQ && m_VortexEnergy > 5.0f) {
            m_VortexActive = true;
            m_VortexEnergy = std::max(0.0f, m_VortexEnergy - 30.0f * dt);

            // Захват пуль в радиусе 2 клеток
            float shieldRadiusSq = 4.0f;
            for (auto& b : gs.bullets) {
                if (!b.isAlive) continue;
                float bdx = b.current.x - gs.titan.position.x;
                float bdy = b.current.y - gs.titan.position.y;
                if ((bdx * bdx + bdy * bdy) <= shieldRadiusSq) {
                    b.isAlive = false;
                    m_CaughtBulletsCount++;
                }
            }
        } else {
            // Выброс пойманных пуль по вектору мыши
            if (m_VortexActive && m_CaughtBulletsCount > 0) {
                float mdx = gs.mouseWorldPos.x - gs.titan.position.x;
                float mdy = gs.mouseWorldPos.y - gs.titan.position.y;
                float mLenSq = mdx * mdx + mdy * mdy;

                if (mLenSq > 0.01f) {
                    float mLen = std::sqrt(mLenSq);
                    Vector3D returnDir = {mdx / mLen, mdy / mLen, 0.0f};

                    for (int i = 0; i < m_CaughtBulletsCount; ++i) {
                        Bullet rb;
                        rb.start   = gs.titan.position;
                        rb.current = gs.titan.position;
                        rb.type    = BulletType::Standard;
                        rb.speed   = 28.0f;
                        // Веер с 25% разбросом
                        float spread = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 0.25f;
                        rb.direction = {returnDir.x + spread, returnDir.y + spread, 0.0f};
                        gs.bullets.push_back(rb);
                    }

                    std::cout << "[BT-7274] Вортекс выброс: " << m_CaughtBulletsCount << " снарядов!" << std::endl;
                }
                m_CaughtBulletsCount = 0;
            }

            m_VortexActive = false;
            m_VortexEnergy = std::min(100.0f, m_VortexEnergy + 15.0f * dt);
        }
    }

    // ═══════════════════════════════════════════════
    // ИИ-решение: стоит ли активировать Core Overdrive
    // ═══════════════════════════════════════════════
    bool validateCoreOverdriveTrigger(const GameState& gs) {
        if (m_CoreChargePercent < 100.0f) return false;
        if (m_CoreOverdriveActive) return false;

        // Перегруз эффективен если есть враги в радиусе и давление достаточное
        int nearbyEnemies = 0;
        for (const auto& e : gs.enemies) {
            if (!e.isAlive) continue;
            float dx = e.position.x - gs.titan.position.x;
            float dy = e.position.y - gs.titan.position.y;
            if ((dx * dx + dy * dy) <= 225.0f)  // 15 клеток
                nearbyEnemies++;
        }

        // Активировать если много врагов рядом или критическая ситуация
        return nearbyEnemies >= 3 || gs.titan.health < gs.titan.maxHealth * 0.3f;
    }

    void executeCoreOverdrive() {
        if (m_CoreChargePercent < 100.0f) return;
        m_CoreOverdriveActive = true;
        m_BoilerSteamPressure = 1200.0f;
        m_CoolantTemperature = 980.0f;
        std::cout << "[BT-7274] !! CORE OVERDRIVE АКТИВИРОВАН !! Давление: 1200 Бар!" << std::endl;
    }

    // ═══════════════════════════════════════════════
    // Автономный режим: Anti-Blocking + Combat Anchor
    // ═══════════════════════════════════════════════
    void updateAutonomousMode(GameState& gs, float dt) {
        float tdx = gs.playerPos.x - gs.titan.position.x;
        float tdy = gs.playerPos.y - gs.titan.position.y;
        float distSq = tdx * tdx + tdy * tdy;

        // Anti-Blocking: уступает дорогу Пилоту (реверс гидравлики)
        if (distSq < m_ExclusionRadiusSq) {
            float dist = std::sqrt(distSq);
            if (dist > 0.001f) {
                Vector3D push = {tdx / dist, tdy / dist, 0.0f};
                float nextX = gs.titan.position.x - push.x * 12.0f * dt;
                float nextY = gs.titan.position.y - push.y * 12.0f * dt;

                if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
                    gs.titan.position.x = nextX;
                if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
                    gs.titan.position.y = nextY;
            }
        } else {
            // Combat Anchor: движение к тактической точке
            Vector3D anchor = calculateCombatAnchor(gs);
            float adx = anchor.x - gs.titan.position.x;
            float ady = anchor.y - gs.titan.position.y;
            float aLenSq = adx * adx + ady * ady;

            if (aLenSq > 1.0f) {
                float aLen = std::sqrt(aLenSq);
                float nextX = gs.titan.position.x + (adx / aLen) * gs.titan.speed * dt;
                float nextY = gs.titan.position.y + (ady / aLen) * gs.titan.speed * dt;

                if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
                    gs.titan.position.x = nextX;
                if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
                    gs.titan.position.y = nextY;
            }
        }
    }

    // ═══════════════════════════════════════════════
    // Симбиотический режим: Пилот управляет БТ
    // Движение относительно камеры (как у Scout)
    // ═══════════════════════════════════════════════
    void updatePilotedMode(GameState& gs, const InputSnapshot& input, float dt) {
        // Оси камеры для Танка — от Танка к мыши
        float camDx = gs.mouseWorldPos.x - gs.titan.position.x;
        float camDy = gs.mouseWorldPos.y - gs.titan.position.y;
        float camLen = std::sqrt(camDx * camDx + camDy * camDy);

        Vector3D camForward = {0.707f, -0.707f, 0.0f};
        Vector3D camRight   = {0.707f,  0.707f, 0.0f};

        if (camLen > 0.01f) {
            camForward.x = camDx / camLen;
            camForward.y = camDy / camLen;
            camRight.x   =  camForward.y;
            camRight.y   = -camForward.x;
        }

        Vector3D moveInput = {0.0f, 0.0f, 0.0f};
        moveInput.x = camForward.x * input.moveForward + camRight.x * input.moveStrafe;
        moveInput.y = camForward.y * input.moveForward + camRight.y * input.moveStrafe;

        float inputLenSq = moveInput.x * moveInput.x + moveInput.y * moveInput.y;
        Vector3D targetVel = {0.0f, 0.0f, 0.0f};

        if (inputLenSq > 0.001f) {
            float len = std::sqrt(inputLenSq);
            // Вортекс замедляет шасси до 40%
            float activeSpeed = m_VortexActive ? (gs.titan.speed * 0.4f) : gs.titan.speed;
            // Повреждённые гусеницы (<40%) → 30% скорости
            if (gs.titan.systems.tracksCondition < 40.0f) activeSpeed *= 0.3f;

            targetVel.x = (moveInput.x / len) * activeSpeed;
            targetVel.y = (moveInput.y / len) * activeSpeed;
        }

        // Тяжёлая инерция 40-тонной машины
        float lerpAcc = (inputLenSq > 0.001f) ? m_TitanAcceleration : m_TitanDeceleration;
        m_Velocity.x += (targetVel.x - m_Velocity.x) * lerpAcc * dt;
        m_Velocity.y += (targetVel.y - m_Velocity.y) * lerpAcc * dt;

        float nextX = gs.titan.position.x + m_Velocity.x * dt;
        float nextY = gs.titan.position.y + m_Velocity.y * dt;

        if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
            gs.titan.position.x = nextX;
        if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
            gs.titan.position.y = nextY;

        // Привязываем позицию игрока к кабине
        gs.playerPos = gs.titan.position;
    }

    // ═══════════════════════════════════════════════
    // Combat Anchor: 40% между Пилотом и центром Роя
    // ═══════════════════════════════════════════════
    Vector3D calculateCombatAnchor(const GameState& gs) {
        if (gs.enemies.empty()) return gs.playerPos;

        Vector3D center = {0.0f, 0.0f, 0.0f};
        int count = 0;

        for (const auto& e : gs.enemies) {
            if (!e.isAlive) continue;
            center.x += e.position.x;
            center.y += e.position.y;
            count++;
        }

        if (count == 0) return gs.playerPos;

        center.x /= count;
        center.y /= count;

        return {
            gs.playerPos.x + (center.x - gs.playerPos.x) * 0.40f,
            gs.playerPos.y + (center.y - gs.playerPos.y) * 0.40f,
            0.0f
        };
    }
};

}  // namespace bunker
