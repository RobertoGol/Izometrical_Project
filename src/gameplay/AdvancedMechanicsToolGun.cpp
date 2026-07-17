#include "gameplay/AdvancedMechanics.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 5) REACTIVE WORLD SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    // ═══════════════════════════════════════════════════════════════════════════════
    // 11) PREFAB LIBRARY & TOOLGUN
    // ═══════════════════════════════════════════════════════════════════════════════

    PrefabLibrary::PrefabLibrary()
    {
        m_Prefabs.push_back({"small_bunker_room", {"#####", "#...#", "#.c.#", "#...#", "#####"}});
        m_Prefabs.push_back({"garage_checkpoint", {"#######", "#.....#", "#..c..#", "#.....#", "###.###"}});
        m_Prefabs.push_back({"defense_corner", {"###", "#t.", "#.."}});

        // Внутриигровые рекламные постеры и вывески из Fallout 76 UI Art Collection:
        m_Prefabs.push_back({"big_freds_bbq_shack", {"######", "#....#", "#.c..#", "######"}});
        m_Prefabs.push_back({"nuka_speedway_hotrod", {"####", "#..#", "####"}});
        m_Prefabs.push_back({"big_als_tattoo_parlor", {"#####", "#...#", "#####"}});
    }

    const PrefabDef* PrefabLibrary::get(const std::string& name) const
    {
        for (const auto& p : m_Prefabs)
        {
            if (p.name == name)
            {
                return &p;
            }
        }
        return m_Prefabs.empty() ? nullptr : &m_Prefabs.front();
    }

    void ToolGunSystem::cycleMode()
    {
        switch (m_Mode)
        {
        case ToolGunMode::SpawnPrefab:
            m_Mode = ToolGunMode::Delete;
            break;
        case ToolGunMode::Delete:
            m_Mode = ToolGunMode::PaintErosion;
            break;
        case ToolGunMode::PaintErosion:
            m_Mode = ToolGunMode::Validate;
            break;
        case ToolGunMode::Validate:
            m_Mode = ToolGunMode::Export;
            break;
        default:
            m_Mode = ToolGunMode::SpawnPrefab;
            break;
        }
    }

    bool ToolGunSystem::apply(GameState& gs, Vector3D where, const PrefabLibrary& lib)
    {
        const int x = static_cast<int>(std::floor(where.x));
        const int y = static_cast<int>(std::floor(where.y));
        if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
        {
            return false;
        }
        switch (m_Mode)
        {
        case ToolGunMode::SpawnPrefab:
            return spawnPrefab(gs, x, y, lib);
        case ToolGunMode::Delete:
            return deleteTile(gs, x, y);
        case ToolGunMode::PaintErosion:
            return paintErosion(gs, x, y);
        case ToolGunMode::Validate:
            m_LastValidation = validate(gs);
            bunker::logInfo() << "[TOOLGUN] " << m_LastValidation << std::endl;
            return true;
        case ToolGunMode::Export:
        {
            m_LastExport = exportMap(gs);
            std::ofstream out("saves/exported_map_level.cfg");
            if (out.is_open())
            {
                out << m_LastExport;
                out.close();
                bunker::logInfo() << "[TOOLGUN EXPORT] Карта уровня экспортирована в saves/exported_map_level.cfg!"
                                  << std::endl;
            }
            return true;
        }
        default:
            break;
        }
        return false;
    }

    bool ToolGunSystem::undo(GameState& gs)
    {
        if (m_Undo.empty())
        {
            return false;
        }
        auto action = std::move(m_Undo.back());
        m_Undo.pop_back();
        action.undo(gs);
        m_Redo.push_back(std::move(action));
        return true;
    }

    bool ToolGunSystem::redo(GameState& gs)
    {
        if (m_Redo.empty())
        {
            return false;
        }
        auto action = std::move(m_Redo.back());
        m_Redo.pop_back();
        action.redo(gs);
        m_Undo.push_back(std::move(action));
        return true;
    }

    void ToolGunSystem::pushAction(ToolGunAction a)
    {
        m_Undo.push_back(std::move(a));
        m_Redo.clear();
        if (m_Undo.size() > 64)
        {
            m_Undo.erase(m_Undo.begin());
        }
    }

    bool ToolGunSystem::spawnPrefab(GameState& gs, int x, int y, const PrefabLibrary& lib)
    {
        const PrefabDef* prefab = lib.get("small_bunker_room");
        if (!prefab)
        {
            return false;
        }
        auto oldMap = gs.sectorMap;
        auto oldDur = gs.wallDurability;
        for (int row = 0; row < static_cast<int>(prefab->rows.size()); ++row)
        {
            for (int col = 0; col < static_cast<int>(prefab->rows[row].size()); ++col)
            {
                const int tx = x + col;
                const int ty = y + row;
                if (tx < 0 || ty < 0 || tx >= Config::MAP_WIDTH || ty >= Config::MAP_HEIGHT)
                {
                    continue;
                }
                const char c = prefab->rows[row][col];
                switch (c)
                {
                case '#':
                    gs.sectorMap[tx][ty] = 1;
                    gs.wallDurability[tx][ty] = 110;
                    break;
                case '.':
                    gs.sectorMap[tx][ty] = 0;
                    gs.wallDurability[tx][ty] = 0;
                    break;
                case 'c':
                {
                    LootContainer lc;
                    lc.position = {tx + 0.5f, ty + 0.5f, 0.0f};
                    lc.type = LootContainerType::WoodenCrate;
                    gs.lootContainers.push_back(lc);
                    break;
                }
                default:
                    break;
                }
            }
        }
        auto newMap = gs.sectorMap;
        auto newDur = gs.wallDurability;
        pushAction({"spawn prefab",
                    [oldMap, oldDur](GameState& s)
                    {
                        s.sectorMap = oldMap;
                        s.wallDurability = oldDur;
                    },
                    [newMap, newDur](GameState& s)
                    {
                        s.sectorMap = newMap;
                        s.wallDurability = newDur;
                    }});
        return true;
    }

    bool ToolGunSystem::deleteTile(GameState& gs, int x, int y)
    {
        const int oldTile = gs.sectorMap[x][y];
        const int oldDur = gs.wallDurability[x][y];
        gs.sectorMap[x][y] = 0;
        gs.wallDurability[x][y] = 0;
        pushAction({"delete tile",
                    [x, y, oldTile, oldDur](GameState& s)
                    {
                        s.sectorMap[x][y] = oldTile;
                        s.wallDurability[x][y] = oldDur;
                    },
                    [x, y](GameState& s)
                    {
                        s.sectorMap[x][y] = 0;
                        s.wallDurability[x][y] = 0;
                    }});
        return true;
    }

    bool ToolGunSystem::paintErosion(GameState& gs, int x, int y)
    {
        const float old = gs.etherErosionMap[x][y];
        gs.etherErosionMap[x][y] = advClamp(old + 10.0f, 0.0f, 100.0f);
        const float now = gs.etherErosionMap[x][y];
        pushAction({"paint erosion", [x, y, old](GameState& s) { s.etherErosionMap[x][y] = old; },
                    [x, y, now](GameState& s) { s.etherErosionMap[x][y] = now; }});
        return true;
    }

    std::string ToolGunSystem::validate(const GameState& gs) const
    {
        int walls = 0;
        int brokenWalls = 0;
        int invalidEnemies = 0;
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                if (gs.sectorMap[x][y] == 1)
                {
                    ++walls;
                    if (gs.wallDurability[x][y] <= 0)
                    {
                        ++brokenWalls;
                    }
                }
            }
        }
        for (const auto& e : gs.enemies)
        {
            const int ex = static_cast<int>(e.position.x);
            const int ey = static_cast<int>(e.position.y);
            if (ex < 0 || ey < 0 || ex >= Config::MAP_WIDTH || ey >= Config::MAP_HEIGHT || gs.sectorMap[ex][ey] == 1)
            {
                ++invalidEnemies;
            }
        }

        bool playerBlocked = false;
        int px = static_cast<int>(gs.playerPos.x);
        int py = static_cast<int>(gs.playerPos.y);
        if (px >= 0 && px < Config::MAP_WIDTH && py >= 0 && py < Config::MAP_HEIGHT)
        {
            playerBlocked = (gs.sectorMap[px][py] == 1);
        }

        std::ostringstream out;
        out << "VALIDATION: walls=" << walls << " brokenDurability=" << brokenWalls
            << " invalidEnemies=" << invalidEnemies
            << (playerBlocked ? " [CRITICAL: PLAYER STUCK IN WALL!]" : " [GRID OK]");
        return out.str();
    }

    std::string ToolGunSystem::exportMap(const GameState& gs) const
    {
        std::ostringstream out;
        out << "# Bunker Protocol ISO map export\n";
        for (int y = 0; y < Config::MAP_HEIGHT; ++y)
        {
            for (int x = 0; x < Config::MAP_WIDTH; ++x)
            {
                out << (gs.sectorMap[x][y] == 1 ? '#' : '.');
            }
            out << '\n';
        }
        return out.str();
    }

    // ═══════════════════════════════════════════════════════════════════════════════


} // namespace bunker
