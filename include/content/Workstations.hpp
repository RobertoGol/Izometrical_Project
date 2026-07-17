#pragma once

#include <string>
#include <vector>

namespace bunker
{
    enum class WorkstationType
    {
        None = 0,
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
        None = 0,
        Craft,
        Repair,
        Modify,
        Scrap,
        Diagnose,
        AccessTerminal,
        OpenStorage
    };

    enum class StationAvailability
    {
        Available,
        Inactive,
        Destroyed,
        Unpowered
    };

    struct WorkstationDef
    {
        int objectID = 0;
        std::string codename;
        std::string displayName;
        WorkstationType stationType = WorkstationType::None;
        std::vector<StationActionType> supportedActions;
        bool isInteractable = true;
        std::string category;
        int requiredScrap = 0;
        int requiredCircuits = 0;
        int requiredCoreEnergy = 0;
        int widthTiles = 1;
        int heightTiles = 1;
        bool requiresPower = false;
        int powerDraw = 0;
        float collisionWidth = 1.0f;
        float collisionHeight = 1.0f;
        float interactionRadius = 1.0f;
        std::string spriteTag;
        std::string uiTitle;
        std::string description;
    };

    struct WorldWorkstation
    {
        int objectID = 0;
        bool isDestroyed = false;
        bool isActive = true;
        bool isPowered = true;
    };

    class WorkstationDatabase
    {
      public:
        WorkstationDatabase();

        void initialize();
        const WorkstationDef* findByID(int objectID) const;
        const WorkstationDef* findByType(WorkstationType stationType) const;

      private:
        std::vector<WorkstationDef> m_Workstations;
    };
} // namespace bunker
