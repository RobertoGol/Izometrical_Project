#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include "IsoMath.hpp"
#include <vector>
#include <cmath>

class Bullet {
public:
    sf::Vector2f worldPos;
    sf::Vector2f velocity;
    bool active = true;

    Bullet(sf::Vector2f start, sf::Vector2f target) {
        worldPos = start;
        sf::Vector2f dir = target - start;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        velocity = (dir / len) * 0.3f; // Скорость пули
    }

    void update() { worldPos += velocity; }
};

class Player {
public:
    sf::Vector2f pos = {5.0f, 5.0f};
    std::vector<Bullet> bullets;

    void update(sf::RenderWindow& window) {
        // Движение
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { pos.x -= Config::PLAYER_SPEED; pos.y -= Config::PLAYER_SPEED; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { pos.x += Config::PLAYER_SPEED; pos.y += Config::PLAYER_SPEED; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { pos.x -= Config::PLAYER_SPEED; pos.y += Config::PLAYER_SPEED; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { pos.x += Config::PLAYER_SPEED; pos.y -= Config::PLAYER_SPEED; }

        // Обновление пуль
        for (auto& b : bullets) b.update();
    }

    void shoot(sf::Vector2f targetWorld) {
        bullets.emplace_back(pos, targetWorld);
    }
};

#endif