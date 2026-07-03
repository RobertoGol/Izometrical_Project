#include "WorkstationDatabase.hpp"

std::unordered_map<WorkstationType, WorkstationDef> WorkstationDatabase::database = {
    {WorkstationType::ArmorWorkbench, {WorkstationType::ArmorWorkbench, "Armor Workbench", {StationActionType::Craft, StationActionType::Repair, StationActionType::Modify, StationActionType::Scrap}}},
    {WorkstationType::WeaponsWorkbench, {WorkstationType::WeaponsWorkbench, "Weapons Workbench", {StationActionType::Craft, StationActionType::Repair, StationActionType::Modify, StationActionType::Scrap}}},
    {WorkstationType::TinkerWorkbench, {WorkstationType::TinkerWorkbench, "Tinker Workbench", {StationActionType::Craft, StationActionType::Scrap}}},
    {WorkstationType::ChemStation, {WorkstationType::ChemStation, "Chemical Station", {StationActionType::Craft}}},
    {WorkstationType::TankMaintenanceBay, {WorkstationType::TankMaintenanceBay, "Tank Maintenance Bay", {StationActionType::Repair, StationActionType::Modify, StationActionType::Diagnose}}},
    {WorkstationType::Terminal, {WorkstationType::Terminal, "Terminal", {StationActionType::AccessTerminal, StationActionType::Diagnose}}},
    {WorkstationType::Storage, {WorkstationType::Storage, "Storage", {StationActionType::OpenStorage}}}};

const WorkstationDef *WorkstationDatabase::findByType(WorkstationType type)
{
    auto it = database.find(type);
    return (it != database.end()) ? &it->second : nullptr;
}