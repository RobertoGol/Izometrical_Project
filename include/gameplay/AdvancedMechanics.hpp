#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Inventory.hpp"
#include "InputManager.hpp"
#include "Constants.hpp"
#include "gameplay/DamageSystem.hpp"
#include "world/WeatherSystem.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // AdvancedMechanics.hpp
    // Единый перенос крупных механик из GMyGameDoNotTouch + Video_Game_Izom.
    //
    // Заголовочный файл содержит строго объявления классов, структур и прототипы
    // методов. Реализация методов физически вынесена в src/gameplay/AdvancedMechanics.cpp.
    // ═══════════════════════════════════════════════════════════════════════════════

    inline float advClamp(float v, float lo, float hi)
    {
        return std::max(lo, std::min(hi, v));
    }

    inline float advDistSq(const Vector3D &a, const Vector3D &b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    inline float advDist2D(const Vector3D &a, const Vector3D &b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    inline Vector3D advNormalize2D(Vector3D v)
    {
        const float len = std::sqrt(v.x * v.x + v.y * v.y);
        if (len < 0.0001f)
            return {0.0f, 0.0f, 0.0f};
        return {v.x / len, v.y / len, 0.0f};
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 2) RADIO / TAPE SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    struct TapeRecord
    {
        std::string id;
        std::string title;
        std::string transcript;
        bool found = false;
        bool played = false;
    };

    struct RadioMessage
    {
        float atTime = 0.0f;
        std::string channel;
        std::string text;
        bool fired = false;
    };

    class RadioTapeSystem
    {
    public:
        RadioTapeSystem();
        void update(float dt);
        void discoverTape(const std::string &id);
        std::string playNextUnplayed();

        const std::string &lastSubtitle() const { return m_LastSubtitle; }
        const std::deque<std::string> &log() const { return m_Log; }
        const std::vector<TapeRecord> &tapes() const { return m_Tapes; }

    private:
        float m_Time = 0.0f;
        std::string m_LastSubtitle;
        std::deque<std::string> m_Log;
        std::vector<TapeRecord> m_Tapes;
        std::vector<RadioMessage> m_Radio;
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 3) RATIONS / HEAL / RELOAD
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class RationKind
    {
        Protein,
        Stamina,
        Focus,
        AntiErosion
    };

    struct ActiveBuff
    {
        RationKind kind = RationKind::Protein;
        float timeLeft = 0.0f;
        float amount = 0.0f;
    };

    struct WeaponRuntime
    {
        int magazine = 30;
        int magazineMax = 30;
        int reserveAmmo = 120;
        float reloadTimer = 0.0f;
        bool isReloading = false;
    };

    class SurvivalSystem
    {
    public:
        static constexpr unsigned int ITEM_STIM = 303;
        static constexpr unsigned int ITEM_RATION_PROTEIN = 510;
        static constexpr unsigned int ITEM_RATION_STAMINA = 511;
        static constexpr unsigned int ITEM_RATION_FOCUS = 512;
        static constexpr unsigned int ITEM_RATION_ANTI_EROSION = 513;
        static constexpr unsigned int ITEM_AMMO_556 = 610;

        void update(GameState &gs, PlayerInventory &inv, float dt);
        bool useStim(GameState &gs, PlayerInventory &inv);
        bool eatRation(GameState &gs, PlayerInventory &inv, RationKind kind);
        void startReload(PlayerInventory &inv);
        bool consumeRound();

        void addStress(float amount) { m_Stress = advClamp(m_Stress + amount, 0.0f, 100.0f); }
        void calm(float amount) { m_Stress = advClamp(m_Stress - amount, 0.0f, 100.0f); }

        float stress() const { return m_Stress; }
        bool secondWindAvailable() const { return !m_SecondWindUsed; }
        bool soulLineAvailable() const { return !m_SoulLineUsed; }
        const WeaponRuntime &weapon() const { return m_Weapon; }
        const std::vector<ActiveBuff> &buffs() const { return m_Buffs; }

        float speedMultiplier() const;
        float aimPenalty() const;

    private:
        float m_Stress = 0.0f;
        bool m_SecondWindUsed = false;
        bool m_SoulLineUsed = false;
        WeaponRuntime m_Weapon;
        std::vector<ActiveBuff> m_Buffs;

        float reloadDuration() const;
        void updateStress(GameState &gs, float dt);
        void updateBuffs(GameState &gs, float dt);
        void updateReload(GameState &gs, float dt);
        void updateSecondWindAndSoulLine(GameState &gs);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 4) TANK UTILITIES / HANGAR / THERMAL LOAD / SEAT SWAP
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class TankUtilityMode
    {
        BucketRig,
        RamShield,
        TowCoupler
    };
    enum class TankSeat
    {
        Driver,
        Gunner
    };

    struct MuzzleSpark
    {
        Vector3D pos;
        Vector3D vel;
        float ttl = 0.15f;
    };

    struct TankUtilityRuntime
    {
        TankUtilityMode utility = TankUtilityMode::BucketRig;
        TankSeat seat = TankSeat::Driver;
        float utilityCooldown = 0.0f;
        float cannonThermalLoad = 0.0f; // 0..100
        bool overheated = false;
        bool towing = false;
        Vector3D towAnchor;
    };

    class TankUtilitySystem
    {
    public:
        static constexpr unsigned int ITEM_REPAIR_KIT = 401;

        void update(GameState &gs, float dt);
        void nextUtility();
        void swapSeat();
        bool useUtility(GameState &gs);
        bool registerCannonShot(GameState &gs, float heat);
        bool repairInHangar(GameState &gs, PlayerInventory &inv);

        bool isInsideHangar(const Vector3D &p) const { return p.x >= 1.5f && p.x <= 6.5f && p.y >= 1.5f && p.y <= 6.5f; }
        const TankUtilityRuntime &runtime() const { return m_Runtime; }
        const std::vector<MuzzleSpark> &sparks() const { return m_Sparks; }

    private:
        TankUtilityRuntime m_Runtime;
        std::vector<MuzzleSpark> m_Sparks;

        void carveFront(GameState &gs, int radius, int damage);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 5) REACTIVE BREAKABLES + SHOCK WAVES
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class BreakableKind
    {
        Glass,
        Vegetation,
        Crate,
        Barrel,
        Console
    };

    struct BreakableObject
    {
        int id = 0;
        BreakableKind kind = BreakableKind::Crate;
        Vector3D position;
        float health = 30.0f;
        float radius = 0.35f;
        bool broken = false;
        Vector3D velocity;
    };

    struct ShockWave
    {
        Vector3D origin;
        float radius = 0.1f;
        float maxRadius = 4.0f;
        float force = 1.0f;
        float ttl = 0.45f;
    };

    class ReactiveWorldSystem
    {
    public:
        int add(BreakableKind kind, Vector3D pos);
        void seedDefault();
        void update(GameState &gs, float dt);
        void damageAt(GameState &gs, Vector3D pos, float radius, float damage, float impulse);

        const std::vector<BreakableObject> &breakables() const { return m_Breakables; }
        const std::vector<ShockWave> &waves() const { return m_Waves; }

    private:
        int m_NextId = 0;
        std::vector<BreakableObject> m_Breakables;
        std::vector<ShockWave> m_Waves;

        void explodeBarrel(GameState &gs, BreakableObject &b);
        void applyWave(GameState &gs, const ShockWave &wave, float dt);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 6) STORY ROUTE / ZONE EVENTS
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class ZoneEventId
    {
        CryoLocker,
        Archive,
        Garage,
        Exterior,
        FirstCombat,
        ReturnToBase,
        SurfaceClearance
    };

    struct ZoneTrigger
    {
        ZoneEventId id = ZoneEventId::CryoLocker;
        Vector3D center;
        float radius = 1.0f;
        bool fired = false;
        std::string objective;
    };

    class StoryRouteSystem
    {
    public:
        StoryRouteSystem();
        void update(GameState &gs, RadioTapeSystem *radio = nullptr);

        const std::vector<StoryRouteEntry> &route() const { return m_Route; }
        const std::string &lastEvent() const { return m_LastEvent; }

    private:
        std::vector<ZoneTrigger> m_Triggers;
        std::vector<StoryRouteEntry> m_Route;
        std::string m_LastEvent;

        void rebuildRoute();
        void mark(const std::string &text);
        void fire(GameState &gs, ZoneTrigger &t, RadioTapeSystem *radio);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 7) SKILLS / XP
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class SkillId
    {
        ArchiveSync,
        FootKill,
        TankAction,
        StressSurvival,
        Mechanic,
        Builder
    };

    struct SkillRuntime
    {
        std::map<SkillId, int> rank;
        int pendingKillsFoot = 0;
        int pendingKillsTank = 0;
    };

    class SkillSystem
    {
    public:
        SkillSystem();
        void grantXp(GameState &gs, int xp);
        bool upgrade(GameState &gs, SkillId id);

        float footDamageMultiplier() const { return 1.0f + 0.08f * rank(SkillId::FootKill); }
        float tankDamageMultiplier() const { return 1.0f + 0.10f * rank(SkillId::TankAction); }
        float repairMultiplier() const { return 1.0f + 0.12f * rank(SkillId::Mechanic); }
        float buildCostMultiplier() const { return 1.0f - 0.07f * rank(SkillId::Builder); }
        int rank(SkillId id) const
        {
            auto it = m_Skills.rank.find(id);
            return it == m_Skills.rank.end() ? 0 : it->second;
        }
        const SkillRuntime &runtime() const { return m_Skills; }

    private:
        SkillRuntime m_Skills;

        static int xpForNext(int level) { return 250 + level * 125; }
        void applyPassive(GameState &gs, SkillId id, int r);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 8) LOOT GENERATOR BY TIERS
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class LootTier
    {
        Common,
        Uncommon,
        Rare,
        Epic,
        Legendary
    };

    struct LootRollEntry
    {
        InventoryItem item;
        int minQty = 1;
        int maxQty = 1;
        float weight = 1.0f;
    };

    class LootGenerator
    {
    public:
        LootGenerator();
        InventoryItem roll(LootTier tier);
        void fillContainer(LootContainer &c, LootTier tier, int rolls);
        void normalizeWorldLoot(GameState &gs);

    private:
        std::mt19937 m_Rng;
        std::map<LootTier, std::vector<LootRollEntry>> m_Tables;

        void add(LootTier tier, unsigned int id, ItemType type, std::string name, int minQ, int maxQ, float w, float unitWeight);
        void addDefaults();
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 9) C.A.M.P. BUILDING
    // ═══════════════════════════════════════════════════════════════════════════════

    struct CampObject
    {
        int id = 0;
        CampObjectType type = CampObjectType::ConcreteWall;
        int tileX = 0;
        int tileY = 0;
        float health = 100.0f;
    };

    class CampSystem
    {
    public:
        static constexpr unsigned int ITEM_BUILD_MATERIAL = 201;

        void toggle() { m_Enabled = !m_Enabled; }
        bool enabled() const { return m_Enabled; }

        void cycleType();
        void updatePreview(const GameState &gs, const Vector3D &mouseWorld);
        bool place(GameState &gs, PlayerInventory &inv, float costMult = 1.0f);
        void updateTurrets(GameState &gs, float dt);

        const CampPreview &preview() const { return m_Preview; }
        const std::vector<CampObject> &objects() const { return m_Objects; }

    private:
        bool m_Enabled = false;
        int m_NextId = 0;
        CampPreview m_Preview;
        std::vector<CampObject> m_Objects;

        static int baseCost(CampObjectType type);
        bool canPlace(const GameState &gs, int x, int y) const;
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 10) SPATIAL GRID TEMPLATE (Legacy Generic Culling Container)
    // ═══════════════════════════════════════════════════════════════════════════════

    template <typename T>
    class TemplateSpatialGrid
    {
    public:
        explicit TemplateSpatialGrid(float cellSize = 2.0f) : m_CellSize(std::max(0.25f, cellSize)) {}

        void clear() { m_Cells.clear(); }

        void insert(Vector3D pos, T value)
        {
            m_Cells[key(cellX(pos.x), cellY(pos.y))].push_back(value);
        }

        template <typename Fn>
        void query(Vector3D pos, float radius, Fn fn) const
        {
            const int minX = cellX(pos.x - radius);
            const int maxX = cellX(pos.x + radius);
            const int minY = cellY(pos.y - radius);
            const int maxY = cellY(pos.y + radius);
            for (int x = minX; x <= maxX; ++x)
            {
                for (int y = minY; y <= maxY; ++y)
                {
                    auto it = m_Cells.find(key(x, y));
                    if (it == m_Cells.end())
                        continue;
                    for (const auto &value : it->second)
                        fn(value);
                }
            }
        }

    private:
        float m_CellSize;
        std::unordered_map<std::int64_t, std::vector<T>> m_Cells;

        int cellX(float x) const { return static_cast<int>(std::floor(x / m_CellSize)); }
        int cellY(float y) const { return static_cast<int>(std::floor(y / m_CellSize)); }
        static std::int64_t key(int x, int y) { return (static_cast<std::int64_t>(x) << 32) ^ static_cast<unsigned int>(y); }
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 11) TOOLGUN / PREFABS / UNDO-REDO
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class ToolGunMode
    {
        SpawnPrefab,
        Delete,
        PaintErosion,
        Validate,
        Export
    };

    struct PrefabDef
    {
        std::string name;
        std::vector<std::string> rows; // '#' wall, '.' floor, 'c' crate, 't' turret
    };

    struct ToolGunAction
    {
        std::string label;
        std::function<void(GameState &)> undo;
        std::function<void(GameState &)> redo;
    };

    class PrefabLibrary
    {
    public:
        PrefabLibrary();
        const PrefabDef *get(const std::string &name) const;
        const std::vector<PrefabDef> &all() const { return m_Prefabs; }

    private:
        std::vector<PrefabDef> m_Prefabs;
    };

    class ToolGunSystem
    {
    public:
        void cycleMode();
        bool apply(GameState &gs, Vector3D where, const PrefabLibrary &lib);
        bool undo(GameState &gs);
        bool redo(GameState &gs);

        ToolGunMode mode() const { return m_Mode; }
        const std::string &lastValidation() const { return m_LastValidation; }
        const std::string &lastExport() const { return m_LastExport; }

    private:
        ToolGunMode m_Mode = ToolGunMode::SpawnPrefab;
        std::vector<ToolGunAction> m_Undo;
        std::vector<ToolGunAction> m_Redo;
        std::string m_LastValidation;
        std::string m_LastExport;

        void pushAction(ToolGunAction a);
        bool spawnPrefab(GameState &gs, int x, int y, const PrefabLibrary &lib);
        bool deleteTile(GameState &gs, int x, int y);
        bool paintErosion(GameState &gs, int x, int y);
        std::string validate(const GameState &gs) const;
        std::string exportMap(const GameState &gs) const;
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 12) OBJ MODEL LOADER
    // ═══════════════════════════════════════════════════════════════════════════════

    struct ObjVertex
    {
        float x = 0, y = 0, z = 0;
    };
    struct ObjTexCoord
    {
        float u = 0, v = 0;
    };
    struct ObjNormal
    {
        float nx = 0, ny = 0, nz = 0;
    };
    struct ObjFace
    {
        std::array<int, 3> v{{0, 0, 0}};
        std::array<int, 3> vt{{0, 0, 0}};
        std::array<int, 3> vn{{0, 0, 0}};
    };

    struct ObjModel
    {
        std::vector<ObjVertex> vertices;
        std::vector<ObjTexCoord> texCoords;
        std::vector<ObjNormal> normals;
        std::vector<ObjFace> faces;
        bool empty() const { return vertices.empty() || faces.empty(); }
        void computeFlatNormalsIfMissing();
    };

    class ObjModelLoader
    {
    public:
        static ObjModel load(const std::string &path);

    private:
        static int parseFaceIndex(const std::string &token);
        static void parseFullFaceTriplet(const std::string &token, int &v_idx, int &vt_idx, int &vn_idx);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 13) LANLINE SERVICES: lobby/chat/voice/delivery/local squad model
    // ═══════════════════════════════════════════════════════════════════════════════

    struct LanlinePeer
    {
        int id = 0;
        std::string name;
        bool voiceActive = false;
        Vector3D lastKnownPos;
        bool inInterestArea = true;
        unsigned int simulatedSocketFd = 0;
    };

    struct LanlineChatMessage
    {
        int fromPeer = 0;
        std::string text;
        float ttl = 10.0f;
    };

    struct LanlineDelivery
    {
        int id = 0;
        std::string payload;
        Vector3D dropPos;
        float eta = 10.0f;
        bool delivered = false;
    };

    class LanlineServices
    {
    public:
        static constexpr int MAX_COOP_PLAYERS = 20;

        int createLocalLobby(const std::string &playerName);
        int addPeer(const std::string &name);
        int addPeerWithSocket(const std::string &name, Vector3D pos, unsigned int sockFd);
        void sendChat(int fromPeer, const std::string &text);
        void setVoice(int peerId, bool active);
        int requestDelivery(const std::string &payload, Vector3D dropPos);
        void update(GameState &gs, PlayerInventory &inv, float dt);

        void cullInactiveOrDistantPeers(const Vector3D &localPlayerPos, float interestRadius);
        void simulateWinsockUdpHeartbeat(float dt);

        bool connected() const { return m_Connected; }
        const std::vector<LanlinePeer> &peers() const { return m_Peers; }
        const std::vector<LanlineChatMessage> &chat() const { return m_Chat; }
        const std::vector<LanlineDelivery> &deliveries() const { return m_Deliveries; }

    private:
        bool m_Connected = false;
        int m_LobbyId = 0;
        int m_NextLobbyId = 1000;
        int m_NextPeerId = 0;
        int m_NextDeliveryId = 0;
        unsigned int m_NextSocketFd = 100042;
        float m_HeartbeatTimer = 0.0f;
        std::vector<LanlinePeer> m_Peers;
        std::vector<LanlineChatMessage> m_Chat;
        std::vector<LanlineDelivery> m_Deliveries;

        void systemMessage(const std::string &text) { m_Chat.push_back({0, text, 8.0f}); }
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 14) PROFILE / SESSION MIGRATION
    // ═══════════════════════════════════════════════════════════════════════════════

    struct PlayerProfile
    {
        unsigned int version = 16;
        std::string playerName = "Pilot";
        int totalKills = 0;
        int totalDeaths = 0;
        int sessionsPlayed = 0;
        float totalPlayTime = 0.0f;
        std::map<std::string, int> flags;
    };

    class ProfileSessionSystem
    {
    public:
        void startSession(PlayerProfile &p);
        void update(PlayerProfile &p, float dt);
        void registerKill(PlayerProfile &p) { p.totalKills += 1; }
        void registerDeath(PlayerProfile &p) { p.totalDeaths += 1; }
        void migrate(PlayerProfile &p);
        std::string summary(const PlayerProfile &p) const;

    private:
        bool m_Started = false;
        float m_SessionTime = 0.0f;
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 15) FACADE: одна точка входа для main.cpp
    // ═══════════════════════════════════════════════════════════════════════════════

    class AdvancedMechanics
    {
    public:
        void initialize(GameState &gs, PlayerInventory &inv);
        void update(GameState &gs, PlayerInventory &inv, const InputSnapshot &input, float dt);
        void onExplosion(GameState &gs, Vector3D pos, float radius, float damage);

        WeatherSystem weather;
        RadioTapeSystem radio;
        SurvivalSystem survival;
        TankUtilitySystem tankUtility;
        ReactiveWorldSystem reactive;
        StoryRouteSystem story;
        SkillSystem skills;
        LootGenerator loot;
        CampSystem camp;
        PrefabLibrary prefabs;
        ToolGunSystem toolgun;
        LanlineServices lanline;
        ProfileSessionSystem profile;
        PlayerProfile playerProfile;
    };

} // namespace bunker
