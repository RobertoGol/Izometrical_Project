#ifndef ENTITY_MANAGER_HPP
#define ENTITY_MANAGER_HPP

#include <vector>
#include <algorithm>
#include <memory>
#include "IsoMath.hpp"

// Базовый класс для всего, что есть в мире
class DrawableEntity {
public:
    sf::Vector2f worldPos;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual ~DrawableEntity() = default;

    // Ключ для сортировки: в изометрии это сумма X и Y
    float getSortOrder() const { return worldPos.x + worldPos.y; }
};

class EntityManager {
public:
    std::vector<std::shared_ptr<DrawableEntity>> entities;

    void clear() { entities.clear(); }

    void add(std::shared_ptr<DrawableEntity> e) { entities.push_back(e); }

    void renderAll(sf::RenderWindow& window) {
        // СОРТИРОВКА (Алгоритм художника)
        std::sort(entities.begin(), entities.end(), [](const auto& a, const auto& b) {
            return a->getSortOrder() < b->getSortOrder();
        });

        for (auto& e : entities) {
            e->draw(window);
        }
    }
};

#endif