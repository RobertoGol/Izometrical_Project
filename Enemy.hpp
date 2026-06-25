#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <SFML/Graphics.hpp>
#include <cmath>
#include "IsoMath.hpp"
#include "Constants.hpp"

class Enemy {
public:
    sf::Vector2f worldPos;
    float speed = 0.03f;
    float health = 50.0f;
    bool alive = true;

    Enemy(float x, float y) : worldPos(x, y) {}

    void update(const sf::Vector2f& playerWorldPos) {
        // Вычисляем вектор направления к игроку
        sf::Vector2f dir = playerWorldPos - worldPos;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if (dist > 0.1f) { // Чтобы враг не дергался, когда стоит вплотную
            worldPos += (dir / dist) * speed;
        }

        if (health <= 0) alive = false;
    }

    void draw(sf::RenderWindow& window) {
        sf::CircleShape shape(10, 3); // Треугольный враг
        shape.setFillColor(sf::Color(255, 50, 50));
        shape.setOrigin(10, 10);
        shape.setPosition(IsoMath::worldToScreen(worldPos.x, worldPos.y));
        window.draw(shape);
    }
};

#endif