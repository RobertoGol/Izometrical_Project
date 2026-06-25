#pragma once

#include "Types.hpp"
#include "Constants.hpp"
#include <string>
#include <vector>
#include <algorithm>

namespace bunker {

class PlayerInventory {
private:
    std::vector<InventoryItem> m_Slots;
    float m_MaxWeight     = Config::INVENTORY_MAX_WEIGHT;
    float m_CurrentWeight = 0.0f;

public:
    PlayerInventory() = default;

    // ── Добавить предмет (стакание если ID совпадает) ──
    bool addItem(unsigned int id, ItemType type, int count, float weight, const std::string& name) {
        // Проверка перегруза
        float potentialWeight = m_CurrentWeight + (weight * static_cast<float>(count));
        if (potentialWeight > m_MaxWeight)
            return false;

        // Стакание если предмет уже есть
        auto it = std::find_if(m_Slots.begin(), m_Slots.end(),
            [id](const InventoryItem& item) { return item.itemID == id; });

        if (it != m_Slots.end()) {
            it->quantity += count;
        } else {
            m_Slots.push_back({id, type, count, weight, name});
        }

        recalculateWeight();
        return true;
    }

    // ── Удалить предмет ──
    bool removeItem(unsigned int id, int count) {
        auto it = std::find_if(m_Slots.begin(), m_Slots.end(),
            [id](const InventoryItem& item) { return item.itemID == id; });

        if (it == m_Slots.end() || it->quantity < count)
            return false;

        it->quantity -= count;
        if (it->quantity <= 0)
            m_Slots.erase(it);

        recalculateWeight();
        return true;
    }

    // ── Проверить наличие предмета ──
    bool hasItem(unsigned int id, int minCount = 1) const {
        auto it = std::find_if(m_Slots.begin(), m_Slots.end(),
            [id](const InventoryItem& item) { return item.itemID == id; });
        return it != m_Slots.end() && it->quantity >= minCount;
    }

    // ── Получить количество предмета ──
    int getItemCount(unsigned int id) const {
        auto it = std::find_if(m_Slots.begin(), m_Slots.end(),
            [id](const InventoryItem& item) { return item.itemID == id; });
        return (it != m_Slots.end()) ? it->quantity : 0;
    }

    // ── Очистить инвентарь ──
    void clear() {
        m_Slots.clear();
        m_CurrentWeight = 0.0f;
    }

    // ── Выдать дев-комплект ──
    void giveDevKit() {
        clear();
        addItem(999, ItemType::Quest,    1,   0.0f, "DEV DEBUG BACKPACK");
        addItem(101, ItemType::Weapon,   1,   0.0f, "PILOT CARBINE XO-16");
        addItem(102, ItemType::Weapon,   1,   0.0f, "SCORCH THERMITE MORTAR");
        addItem(303, ItemType::Medicine, 99,  0.0f, "STIM INJECTOR v15");
        addItem(401, ItemType::Resource, 500, 0.0f, "BT-7274 REPAIR KIT");
        addItem(777, ItemType::Weapon,   1,   0.0f, "GMOD TOOLGUN");
        addItem(888, ItemType::Weapon,   1,   0.0f, "DEBUGGUN CHAIN LIGHTNING");
    }

    // ── Геттеры ──
    const std::vector<InventoryItem>& getSlots()  const { return m_Slots; }
    float getCurrentWeight() const { return m_CurrentWeight; }
    float getMaxWeight()     const { return m_MaxWeight; }
    float getWeightPercent() const { return (m_MaxWeight > 0.0f) ? m_CurrentWeight / m_MaxWeight : 0.0f; }
    int   getSlotCount()     const { return static_cast<int>(m_Slots.size()); }
    bool  isEmpty()          const { return m_Slots.empty(); }
    bool  isFull()           const { return m_CurrentWeight >= m_MaxWeight; }

private:
    void recalculateWeight() {
        m_CurrentWeight = 0.0f;
        for (const auto& item : m_Slots) {
            m_CurrentWeight += item.weightPerUnit * static_cast<float>(item.quantity);
        }
    }
};

}  // namespace bunker
