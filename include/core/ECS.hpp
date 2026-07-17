#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace bunker {

    using EntityID = std::uint32_t;
    constexpr EntityID MAX_ENTITIES = 50000; // Держим в уме масштаб Fallout 76

    // ЧИСТЫЕ ДАННЫЕ (Components)
    // ---------------------------------------------------------
    struct TransformComponent {
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
    };

    struct MeshComponent {
        std::uint32_t vaoID = 0;       // ID геометрии на видеокарте (Vertex Array Object)
        std::uint32_t indexCount = 0;  // Количество вершин для отрисовки
        std::uint32_t materialID = 0;  // PBR-материал (Albedo, Normal, Roughness, Metallic)
    };

    // ПЛОТНЫЕ МАССИВЫ (Data-Oriented Design)
    // ---------------------------------------------------------
    template<typename T>
    class ComponentArray {
    public:
        void insert(EntityID entity, const T& component) {
            auto existing = entityToIndex.find(entity);
            if (existing != entityToIndex.end()) {
                components[existing->second] = component;
                return;
            }

            entityToIndex[entity] = components.size();
            indexToEntity[components.size()] = entity;
            components.push_back(component);
            denseEntities.push_back(entity);
        }

        T* get(EntityID entity) {
            auto it = entityToIndex.find(entity);
            if (it == entityToIndex.end()) return nullptr;
            return &components[it->second];
        }

        // Ключевой метод для рендера: отдаем сырой массив для линейного прохода
        std::vector<T>& getRawData() { return components; }
        const std::vector<EntityID>& getDenseEntities() const { return denseEntities; }

    private:
        std::vector<T> components;
        std::vector<EntityID> denseEntities; // ID сущностей, синхронизированные с индексами массива
        std::unordered_map<EntityID, size_t> entityToIndex;
        std::unordered_map<size_t, EntityID> indexToEntity;
    };

    // ГЛАВНЫЙ РЕЕСТР СИМУЛЯЦИИ
    // ---------------------------------------------------------
    class Registry {
    public:
        EntityID createEntity() { return nextEntity++; }

        ComponentArray<TransformComponent> transforms;
        ComponentArray<MeshComponent> meshes;
        // В будущем добавишь сюда: Physics, AIState, Health и т.д.

    private:
        EntityID nextEntity = 0;
    };

} // namespace bunker
