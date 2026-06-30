#include "persistence/Inventory.hpp"
#include <algorithm>

namespace bunker
{

    bool PlayerInventory::addItem(unsigned int id, ItemType type, int count, float weight, const std::string &name)
    {
        float potentialWeight = m_CurrentWeight + (weight * static_cast<float>(count));
        if (potentialWeight > m_MaxWeight)
            return false;

        auto it = std::find_if(m_Slots.begin(), m_Slots.end(),
                               [id](const InventoryItem &item)
                               { return item.itemID == id; });

        if (it != m_Slots.end())
        {
            it->quantity += count;
        }
        else
        {
            m_Slots.push_back({id, type, count, weight, name});
        }

        recalculateWeight();
        return true;
    }

    bool PlayerInventory::removeItem(unsigned int id, int count)
    {
        auto it = std::find_if(m_Slots.begin(), m_Slots.end(),
                               [id](const InventoryItem &item)
                               { return item.itemID == id; });

        if (it == m_Slots.end() || it->quantity < count)
            return false;

        it->quantity -= count;
        if (it->quantity <= 0)
            m_Slots.erase(it);

        recalculateWeight();
        return true;
    }

    bool PlayerInventory::hasItem(unsigned int id, int minCount) const
    {
        auto it = std::find_if(m_Slots.begin(), m_Slots.end(),
                               [id](const InventoryItem &item)
                               { return item.itemID == id; });
        return it != m_Slots.end() && it->quantity >= minCount;
    }

    int PlayerInventory::getItemCount(unsigned int id) const
    {
        auto it = std::find_if(m_Slots.begin(), m_Slots.end(),
                               [id](const InventoryItem &item)
                               { return item.itemID == id; });
        return (it != m_Slots.end()) ? it->quantity : 0;
    }

    void PlayerInventory::clear()
    {
        m_Slots.clear();
        m_CurrentWeight = 0.0f;
    }

    void PlayerInventory::giveDevKit()
    {
        clear();
        addItem(999, ItemType::Quest, 1, 0.0f, "DEV DEBUG BACKPACK");
        addItem(101, ItemType::Weapon, 1, 0.0f, "PILOT CARBINE XO-16");
        addItem(102, ItemType::Weapon, 1, 0.0f, "SCORCH THERMITE MORTAR");
        addItem(303, ItemType::Medicine, 99, 0.0f, "STIM INJECTOR v15");
        addItem(401, ItemType::Resource, 500, 0.0f, "BT-7274 REPAIR KIT");
        addItem(777, ItemType::Weapon, 1, 0.0f, "GMOD TOOLGUN");
        addItem(888, ItemType::Weapon, 1, 0.0f, "DEBUGGUN CHAIN LIGHTNING");
    }

    void PlayerInventory::recalculateWeight()
    {
        m_CurrentWeight = 0.0f;
        for (const auto &item : m_Slots)
        {
            m_CurrentWeight += item.weightPerUnit * static_cast<float>(item.quantity);
        }
    }

} // namespace bunker