#pragma once

#include "WorkstationTypes.hpp"
#include <unordered_map>

class WorkstationDatabase
{
public:
    static const WorkstationDef *findByType(WorkstationType type);

private:
    static std::unordered_map<WorkstationType, WorkstationDef> database;
};