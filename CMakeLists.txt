#pragma once

#include "core/Types.hpp"
#include "core/Constants.hpp"
#include <string>
#include <vector>

namespace bunker {

class PlayerInventory {
private:
    std::vector<InventoryItem> m_Slots;
    float m_MaxWeight     = Config::INVENTORY_MAX_WEIGHT;
    float m_CurrentWeight = 0.0f;

public:
    PlayerInventory() = default;

    bool addItem(unsigned int id, ItemType type, int count, float weight, const std::string& name);
    bool removeItem(unsigned int id, int count);
    bool hasItem(unsigned int id, int minCount = 1) const;
    int  getItemCount(unsigned int id) const;
    void clear();
    void giveDevKit();

    const std::vector<InventoryItem>& getSlots()  const { return m_Slots; }
    float getCurrentWeight() const { return m_CurrentWeight; }
    float getMaxWeight()     const { return m_MaxWeight; }
    float getWeightPercent() const { return (m_MaxWeight > 0.0f) ? m_CurrentWeight / m_MaxWeight : 0.0f; }
    int   getSlotCount()     const { return static_cast<int>(m_Slots.size()); }
    bool  isEmpty()          const { return m_Slots.empty(); }
    bool  isFull()           const { return m_CurrentWeight >= m_MaxWeight; }

private:
    void recalculateWeight();
};

}  // namespace bunker
