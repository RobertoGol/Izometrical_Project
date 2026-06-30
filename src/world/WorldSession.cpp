#include "world/WorldSession.hpp"
#include "gameplay/DamageSystem.hpp"
#include "core/Constants.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace bunker
{

    void WorldSession::generateDefaultWorld(GameState &gs, EnemySpawner &spawner)
    {
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                gs.sectorMap[x][y] = 0;
                gs.wallDurability[x][y] = 0;
                gs.etherErosionMap[x][y] = 0.0f;
            }
        }

        for (int x = 5; x < 15; ++x)
        {
            for (int y = 5; y < 15; ++y)
            {
                if (x == 5 || x == 14 || y == 5 || y == 14)
                {
                    gs.sectorMap[x][y] = 1;
                    gs.wallDurability[x][y] = 100;
                }
            }
        }

        gs.sectorMap[9][5] = 0;
        gs.sectorMap[9][14] = 0;
        gs.sectorMap[5][9] = 0;
        gs.sectorMap[14][9] = 0;

        gs.towerPosition = {Config::TOWER_X, Config::TOWER_Y, 0.0f};
        gs.regionalGrid.towerHealth = 200.0f;

        gs.etherErosionMap[2][2] = 15.0f;
        gs.etherErosionMap[17][17] = 30.0f;
        gs.mapMeta.activeVerminNests = 2;

        spawner.spawnTrainingZone(gs);

        gs.bunkerProgression.hasFoundPipPad = false;
        gs.bunkerProgression.pipPadSpawnPos = {Config::PIPPAD_SPAWN_X, Config::PIPPAD_SPAWN_Y, 0.0f};

        gs.playerPos = {5.0f, 5.0f, 0.0f};
        gs.titan.position = {7.0f, 7.0f, 0.0f};

        std::cout << "[WORLD] Убежище 17 сгенерировано." << std::endl;
    }

    void WorldSession::update(GameState &gs, float dt)
    {
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                if (gs.etherErosionMap[x][y] > 0.01f)
                {
                    gs.etherErosionMap[x][y] = std::min(100.0f,
                                                        gs.etherErosionMap[x][y] + Config::EROSION_SPREAD_RATE * dt);

                    float dx = gs.playerPos.x - (static_cast<float>(x) + 0.5f);
                    float dy = gs.playerPos.y - (static_cast<float>(y) + 0.5f);

                    if ((dx * dx + dy * dy) < 0.25f)
                    {
                        gs.playerErosionLevel = std::min(100.0f,
                                                         gs.playerErosionLevel + Config::EROSION_PLAYER_DAMAGE * dt);

                        if (gs.playerErosionLevel > Config::EROSION_DAMAGE_THRESHOLD)
                        {
                            DamageSystem::applyPlayerDamage(gs, 1.5f * dt, DamageType::Environmental);
                        }
                    }
                }
            }
        }

        if (gs.playerErosionLevel > 0.0f)
        {
            bool onErosion = false;
            int px = static_cast<int>(gs.playerPos.x);
            int py = static_cast<int>(gs.playerPos.y);
            if (px >= 0 && px < Config::MAP_WIDTH && py >= 0 && py < Config::MAP_HEIGHT)
            {
                onErosion = gs.etherErosionMap[px][py] > 0.01f;
            }
            if (!onErosion)
            {
                gs.playerErosionLevel = std::max(0.0f, gs.playerErosionLevel - 1.0f * dt);
            }
        }

        if (!gs.mapMeta.isBaseCleared && gs.regionalGrid.towerHealth > 0.0f)
        {
            gs.mapMeta.baseSuppliesLevel = std::max(0.0f,
                                                    gs.mapMeta.baseSuppliesLevel - 0.02f * gs.mapMeta.activeVerminNests * dt);
        }

        if (!gs.bunkerProgression.hasFoundPipPad)
        {
            float dx = gs.playerPos.x - gs.bunkerProgression.pipPadSpawnPos.x;
            float dy = gs.playerPos.y - gs.bunkerProgression.pipPadSpawnPos.y;
            if ((dx * dx + dy * dy) < 0.5f)
            {
                gs.bunkerProgression.hasFoundPipPad = true;
                std::cout << "[STORY] Pip-Pad найден! Тактические способности разблокированы." << std::endl;
            }
        }

        if (gs.regionalGrid.towerHealth > 0.0f)
        {
            for (const auto &e : gs.enemies)
            {
                if (!e.isAlive)
                    continue;
                float dx = e.position.x - gs.towerPosition.x;
                float dy = e.position.y - gs.towerPosition.y;
                if ((dx * dx + dy * dy) < 1.0f)
                {
                    DamageSystem::applyTowerDamage(gs, 5.0f * dt);
                }
            }
        }

        static float spreadAccum = 0.0f;
        spreadAccum += dt;
        if (spreadAccum >= 5.0f)
        {
            spreadAccum = 0.0f;
            spreadErosionToNeighbors(gs);
        }
    }

    void WorldSession::interactWithContainers(GameState &gs, PlayerInventory &inventory)
    {
        for (auto &container : gs.lootContainers)
        {
            if (container.isOpened)
                continue;

            float dx = gs.playerPos.x - container.position.x;
            float dy = gs.playerPos.y - container.position.y;

            if ((dx * dx + dy * dy) < 1.0f)
            {
                container.isOpened = true;

                for (const auto &item : container.containsItems)
                {
                    inventory.addItem(item.itemID, item.type, item.quantity,
                                      item.weightPerUnit, item.displayName);
                }

                std::cout << "[LOOT] Контейнер вскрыт, получено "
                          << container.containsItems.size() << " предметов." << std::endl;
                return;
            }
        }
    }

    void WorldSession::spreadErosionToNeighbors(GameState &gs)
    {
        std::array<std::array<float, Config::MAP_HEIGHT>, Config::MAP_WIDTH> copy = gs.etherErosionMap;

        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                if (copy[x][y] > 20.0f)
                {
                    int neighbors[4][2] = {{x - 1, y}, {x + 1, y}, {x, y - 1}, {x, y + 1}};
                    for (auto &n : neighbors)
                    {
                        int nx = n[0], ny = n[1];
                        if (nx >= 0 && nx < Config::MAP_WIDTH &&
                            ny >= 0 && ny < Config::MAP_HEIGHT)
                        {
                            if (gs.sectorMap[nx][ny] != 1)
                            {
                                gs.etherErosionMap[nx][ny] = std::min(100.0f,
                                                                      gs.etherErosionMap[nx][ny] + copy[x][y] * 0.01f);
                            }
                        }
                    }
                }
            }
        }
    }

} // namespace bunker