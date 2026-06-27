#pragma once

namespace Config {

// ── Экран ──
inline constexpr int SCREEN_WIDTH  = 1280;
inline constexpr int SCREEN_HEIGHT = 720;

// ── Изометрическая сетка ──
inline constexpr float TILE_W = 64.0f;
inline constexpr float TILE_H = 32.0f;

// ── Карта ──
inline constexpr int MAP_WIDTH  = 20;
inline constexpr int MAP_HEIGHT = 20;

// ── Эрозия ──
inline constexpr float EROSION_DAMAGE_THRESHOLD = 50.0f;
inline constexpr float EROSION_SPREAD_RATE      = 0.3f;
inline constexpr float EROSION_PLAYER_DAMAGE    = 2.5f;

// ── Игрок (Пилот) ──
inline constexpr float PLAYER_WALK_SPEED   = 5.5f;
inline constexpr float PLAYER_SPRINT_SPEED = 8.5f;
inline constexpr float PLAYER_START_HP     = 100.0f;
inline constexpr float PLAYER_RADIUS       = 0.25f;
inline constexpr float PLAYER_DIVE_SPEED   = 9.5f;
inline constexpr float PLAYER_DIVE_TIME    = 0.6f;

// ── Стамина ──
inline constexpr float STAMINA_MAX   = 100.0f;
inline constexpr float STAMINA_DRAIN = 30.0f;
inline constexpr float STAMINA_REGEN = 18.0f;

// ── Титан / Танк ──
inline constexpr float TITAN_HP          = 600.0f;
inline constexpr float TITAN_SPEED       = 3.2f;
inline constexpr float TITAN_RADIUS      = 0.55f;
inline constexpr float TITAN_FIRE_RATE   = 0.4f;

// ── Оружие пехотинца ──
inline constexpr float CARBINE_COOLDOWN    = 0.22f;
inline constexpr float SHOTGUN_COOLDOWN    = 0.55f;
inline constexpr int   SHOTGUN_PELLETS     = 5;
inline constexpr float PELLET_SPREAD       = 0.04f;
inline constexpr float BULLET_SPEED        = 28.0f;
inline constexpr float BULLET_MAX_RANGE    = 16.0f;

// ── Ракеты Титана ──
inline constexpr float BALLISTIC_MISSILE_SPEED  = 15.0f;
inline constexpr float BALLISTIC_SPLASH_RADIUS  = 1.8f;
inline constexpr float ARTILLERY_MISSILE_SPEED   = 10.0f;
inline constexpr float ARTILLERY_SPLASH_RADIUS   = 2.4f;
inline constexpr float MISSILE_COOLDOWN          = 1.8f;

// ── Враги (Рой Верминов) ──
inline constexpr float ENEMY_BASE_HP     = 35.0f;
inline constexpr float ENEMY_BASE_SPEED  = 2.4f;
inline constexpr float ENEMY_RADIUS      = 0.35f;
inline constexpr float ENEMY_SPAWN_INTERVAL = 3.5f;
inline constexpr int   ENEMY_MAX_COUNT      = 60;
inline constexpr float ENEMY_AGGRO_RANGE_SQ = 49.0f;  // 7 клеток в квадрате

// ── Камера ──
inline constexpr float CAMERA_BASE_ZOOM    = 55.0f;
inline constexpr float CAMERA_AIM_OFFSET   = 0.35f;
inline constexpr float CAMERA_SMOOTH_SPEED = 5.0f;

// ── Транспорт ──
inline constexpr float STEAMCAR_MAX_PRESSURE  = 240.0f;
inline constexpr float STEAMCAR_MAX_SPEED     = 6.5f;
inline constexpr float MOTORCYCLE_MAX_SPEED   = 11.5f;

// ── Инвентарь ──
inline constexpr float INVENTORY_MAX_WEIGHT = 45.0f;

// ── Тактики (кулдауны в секундах) ──
inline constexpr float GRAPPLE_COOLDOWN     = 9.0f;
inline constexpr float GRAPPLE_DURATION     = 2.0f;
inline constexpr float GRAPPLE_MAX_RANGE_SQ = 144.0f;  // 12 клеток в квадрате
inline constexpr float GRAPPLE_PULL_FORCE   = 8.0f;

inline constexpr float STIM_COOLDOWN        = 14.0f;
inline constexpr float STIM_DURATION        = 3.0f;
inline constexpr float STIM_HEAL_RATE       = 15.0f;

inline constexpr float PHASE_COOLDOWN       = 18.0f;
inline constexpr float PHASE_DURATION       = 2.5f;

inline constexpr float CLOAK_COOLDOWN       = 16.0f;
inline constexpr float CLOAK_DURATION       = 5.5f;

inline constexpr float AWALL_COOLDOWN       = 15.0f;
inline constexpr float AWALL_DURATION       = 5.0f;
inline constexpr float AWALL_HP             = 200.0f;

// ── Башня синхронизации ──
inline constexpr float TOWER_X = 9.5f;
inline constexpr float TOWER_Y = 9.5f;

// ── Pip-Pad ──
inline constexpr float PIPPAD_SPAWN_X = 8.0f;
inline constexpr float PIPPAD_SPAWN_Y = 6.0f;

// ── Прогрессия ──
inline constexpr int   BASE_XP_PER_LEVEL = 100;
inline constexpr int   XP_INCREMENT      = 75;
inline constexpr int   POINTS_PER_LEVEL  = 2;
inline constexpr float HP_PER_LEVEL      = 8.0f;
inline constexpr float MP_PER_LEVEL      = 5.0f;

}  // namespace Config
