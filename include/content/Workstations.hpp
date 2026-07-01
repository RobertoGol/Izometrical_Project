#pragma once

#include "core/Types.hpp"
#include <string>
#include <vector>

namespace bunker
{
    enum class WorkstationType
    {
        ArmorWorkbench,
        WeaponsWorkbench,
        TinkerWorkbench,
        ChemStation,
        TankMaintenanceBay,
        Terminal,
        Storage
    };

    enum class StationActionType
    {
        Craft,
        Repair,
        Modify,
        Scrap,
        Diagnose,
        AccessTerminal,
        OpenStorage
    };

    struct WorkstationDef
    {
        int objectID = 0;
        std::string codename;
        std::string displayName;

        WorkstationType stationType = WorkstationType::ArmorWorkbench;
        std::vector<StationActionType> supportedActions;

        bool canBuildInCamp = true;
        std::string campCategory = "Crafting";

        int buildCostSteel = 0;
        int buildCostWood = 0;
        int buildCostCircuits = 0;
        int buildCostGear = 0;
        int buildCostOil = 0;

        bool requiresPower = false;
        int powerConsumption = 0;

        float footprintWidth = 1.0f;
        float footprintHeight = 1.0f;
        float interactionRadius = 1.5f;

        std::string spriteTag;
        std::string uiTitle;
        std::string loreText;
    };

    struct WorldWorkstation
    {
        int objectID = 0;
        Vector3D position;

        bool isActive = true;
        bool isPowered = true;
        bool isDestroyed = false;

        float currentHealth = 100.0f;
        float maxHealth = 100.0f;
    };

    class WorkstationDatabase
    {
    public:
        WorkstationDatabase();

        void initialize();

        const WorkstationDef *findByID(int objectID) const;
        const std::vector<WorkstationDef> &all() const { return m_Workstations; }

    private:
        std::vector<WorkstationDef> m_Workstations;
    };
}