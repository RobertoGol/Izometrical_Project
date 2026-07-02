#pragma once

#include "content/Workstations.hpp"
#include "persistence/ModularEquipmentSystem.hpp"
#include "persistence/Inventory.hpp"
#include "WorkstationTypes.hpp"
#include <string>
#include <vector>

namespace bunker
{
    // Полноценное состояние интерфейса станции (см. manuals/Doc3.pdf).
    // Это центральный контейнер данных station UI: какая станция открыта,
    // какие действия ей доступны, что выбрано сейчас, и что показать игроку.
    struct WorkstationUIState
    {
        WorkstationType stationType = WorkstationType::ArmorWorkbench;
        int objectID = 0;
        std::string title;
        std::vector<StationActionType> supportedActions;
        int selectedActionIndex = -1;               // -1 = действие ещё не выбрано
        std::vector<CraftingRecipe> visibleRecipes; // только рецепты этой станции
        int selectedRecipeIndex = -1;               // -1 = рецепт ещё не выбран
        std::string statusMessage;                  // результат последнего действия/попытки
        bool isOpen = false;
        StationAvailability availability = StationAvailability::Available;
    };

    // Строит UI-состояние станции по её типу, беря данные из WorkstationDatabase
    // (uiTitle, supportedActions). Если WorkstationDef не найден — используется
    // fallback через switch, чтобы UI не падал и не оставался пустым.
    WorkstationUIState CreateWorkstationUIState(const WorkstationDatabase &database,
                                                ModularEquipmentSystem &equipmentSystem,
                                                WorkstationType stationType);

    // Выбор действия станции по индексу в supportedActions.
    // Сбрасывает выбранный рецепт при смене действия.
    void SelectStationAction(WorkstationUIState &state, int actionIndex);

    // Выбор рецепта по индексу внутри visibleRecipes (только имеет смысл в режиме Craft).
    void SelectRecipe(WorkstationUIState &state, int recipeIndex);

    // ── Обработчики действий станции (StationActionType dispatcher) ──
    // Каждое действие живёт в своей функции, а не в одном большом cout.
    // На этом этапе Repair/Modify/Scrap/Diagnose/AccessTerminal/OpenStorage —
    // это правильно оформленные заглушки: архитектура уже готова принять
    // реальную логику позже, ничего не приходится ломать.
    void HandleCraftAction(WorkstationUIState &state, ModularEquipmentSystem &equipmentSystem, PlayerInventory &inventory);
    void HandleRepairAction(WorkstationUIState &state);
    void HandleModifyAction(WorkstationUIState &state);
    void HandleScrapAction(WorkstationUIState &state);
    void HandleDiagnoseAction(WorkstationUIState &state);
    void HandleAccessTerminalAction(WorkstationUIState &state);
    void HandleOpenStorageAction(WorkstationUIState &state);

    // Центральная развилка station logic: получает текущее выбранное действие
    // и вызывает нужный обработчик. Ничего не выполняется автоматически —
    // вызывается только когда игрок явно подтвердил выполнение действия.
    void ExecuteSelectedAction(WorkstationUIState &state,
                               ModularEquipmentSystem &equipmentSystem,
                               PlayerInventory &inventory);

    // Открытие станции как каркаса: заголовок -> список действий -> (если Craft
    // выбран) список рецептов -> статус. Не выполняет никаких действий сама
    // по себе — ни автокрафта, ни любого другого автозапуска.
    WorkstationUIState OpenWorkstationUI(const WorkstationDatabase &database,
                                         ModularEquipmentSystem &equipmentSystem,
                                         WorkstationType stationType);

    // Полный path из мира: находит WorkstationDef по objectID реального
    // WorldWorkstation, проверяет его состояние (destroyed/inactive/unpowered)
    // и только потом открывает станцию. Это точка входа для player interaction.
    WorkstationUIState OpenWorkstationUIFromWorld(const WorkstationDatabase &database,
                                                  const WorldWorkstation &worldStation,
                                                  ModularEquipmentSystem &equipmentSystem);
}