#pragma once

#include "gameplay/advanced/AdvancedMechanicsCampSystems.hpp"
namespace bunker
{

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
        std::function<void(GameState&)> undo;
        std::function<void(GameState&)> redo;
    };

    class PrefabLibrary
    {
      public:
        PrefabLibrary();
        const PrefabDef* get(const std::string& name) const;
        const std::vector<PrefabDef>& all() const
        {
            return m_Prefabs;
        }

      private:
        std::vector<PrefabDef> m_Prefabs;
    };

    class ToolGunSystem
    {
      public:
        void cycleMode();
        bool apply(GameState& gs, Vector3D where, const PrefabLibrary& lib);
        bool undo(GameState& gs);
        bool redo(GameState& gs);

        ToolGunMode mode() const
        {
            return m_Mode;
        }
        const std::string& lastValidation() const
        {
            return m_LastValidation;
        }
        const std::string& lastExport() const
        {
            return m_LastExport;
        }

      private:
        ToolGunMode m_Mode = ToolGunMode::SpawnPrefab;
        std::vector<ToolGunAction> m_Undo;
        std::vector<ToolGunAction> m_Redo;
        std::string m_LastValidation;
        std::string m_LastExport;

        void pushAction(ToolGunAction a);
        bool spawnPrefab(GameState& gs, int x, int y, const PrefabLibrary& lib);
        bool deleteTile(GameState& gs, int x, int y);
        bool paintErosion(GameState& gs, int x, int y);
        std::string validate(const GameState& gs) const;
        std::string exportMap(const GameState& gs) const;
    };


} // namespace bunker
