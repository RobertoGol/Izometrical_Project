#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm> // Обязательно для remove_if
#include <cmath>
#include "IsoMath.hpp"

class Bullet {
public:
    sf::Vector2f worldPos;
    sf::Vector2f velocity;
    bool active = true;

    Bullet(sf::Vector2f start, sf::Vector2f target) {
        worldPos = start;
        sf::Vector2f dir = target - start;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len != 0) velocity = (dir / len) * 0.5f; 
    }

    void update() {
        worldPos += velocity;
    }
};

class Player {
public:
    sf::Vector2f pos = {5.0f, 5.0f};
    std::vector<Bullet> bullets;

    void update() {
        // Движение (WASD в изометрических осях)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { pos.x -= Config::PLAYER_SPEED; pos.y -= Config::PLAYER_SPEED; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { pos.x += Config::PLAYER_SPEED; pos.y += Config::PLAYER_SPEED; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { pos.x -= Config::PLAYER_SPEED; pos.y += Config::PLAYER_SPEED; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { pos.x += Config::PLAYER_SPEED; pos.y -= Config::PLAYER_SPEED; }

        // Обновление пуль
        for (auto& b : bullets) {
            b.update();
            // Деактивируем пули, улетевшие далеко
            if (std::abs(b.worldPos.x - pos.x) > 20 || std::abs(b.worldPos.y - pos.y) > 20) b.active = false;
        }

        // ПРАВИЛЬНОЕ удаление из вектора (Erase-Remove Idiom)
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), 
            [](const Bullet& b) { return !b.active; }), bullets.end());
    }
	
	// Добавь этот метод внутрь класса Player в Player.hpp
	void checkCollisions(std::vector<Enemy>& enemies) {
		for (auto& b : bullets) {
			if (!b.active) continue;
			for (auto& e : enemies) {
				if (!e.alive) continue;
				
				// Расстояние в мировых координатах
				float dx = b.worldPos.x - e.worldPos.x;
				float dy = b.worldPos.y - e.worldPos.y;
				float dist = std::sqrt(dx*dx + dy*dy);

				if (dist < 0.5f) { // Радиус попадания
					e.health -= 25.0f;
					b.active = false; // Пуля исчезает при попадании
				}
			}
		}
	}
	
    void shoot(sf::Vector2f targetWorld) {
        bullets.emplace_back(pos, targetWorld);
    }
};

#endif