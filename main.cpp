#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include "Player.hpp"
#include "Enemy.hpp"
#include "IsoMath.hpp"

// Структура для сортировки отрисовки
struct RenderObject {
    float depth;
    std::function<void()> drawFunc;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT), "Helldivers ISO Fixed");
    window.setFramerateLimit(60);

    Player player;
    std::vector<Enemy> enemies;
    enemies.emplace_back(2.0f, 2.0f);
    enemies.emplace_back(12.0f, 12.0f);

    sf::View view(sf::Vector2f(0, 0), sf::Vector2f(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT));
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds(); // DeltaTime для плавности
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                player.shoot(IsoMath::screenToWorld(mPos.x, mPos.y));
            }
        }

        // 1. Логика
        player.update(); // Здесь можно добавить dt: player.update(dt)
        for (auto& e : enemies) e.update(player.pos);
        player.checkCollisions(enemies);

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), 
            [](const Enemy& e) { return !e.alive; }), enemies.end());

        // 2. Камера
        view.setCenter(IsoMath::worldToScreen(player.pos.x, player.pos.y));
        window.setView(view);

        window.clear(sf::Color(20, 20, 22));

        // 3. Рендеринг пола (всегда самый нижний слой)
        for (int x = 0; x < 20; x++) {
            for (int y = 0; y < 20; y++) {
                sf::ConvexShape tile(4);
                // Рисуем тайл так, чтобы координаты игрока (5,5) соответствовали ЦЕНТРУ тайла
                tile.setPoint(0, IsoMath::worldToScreen(x, y));
                tile.setPoint(1, IsoMath::worldToScreen(x + 1, y));
                tile.setPoint(2, IsoMath::worldToScreen(x + 1, y + 1));
                tile.setPoint(3, IsoMath::worldToScreen(x, y + 1));
                tile.setFillColor(sf::Color(35, 35, 40));
                tile.setOutlineThickness(1);
                tile.setOutlineColor(sf::Color(50, 50, 55));
                window.draw(tile);
            }
        }

        // 4. СОРТИРОВКА ОБЪЕКТОВ (Z-Sorting)
        // Создаем список всех объектов, которые нужно нарисовать
        std::vector<RenderObject> renderQueue;

        // Добавляем пули (они обычно рисуются под персонажами, но можно и в общую очередь)
        for (auto& b : player.bullets) {
            renderQueue.push_back({ b.worldPos.x + b.worldPos.y, [&]() {
                sf::CircleShape bs(3);
                bs.setFillColor(sf::Color::Yellow);
                bs.setPosition(IsoMath::worldToScreen(b.worldPos.x, b.worldPos.y));
                window.draw(bs);
            }});
        }

        // Добавляем врагов
        for (auto& e : enemies) {
            renderQueue.push_back({ e.worldPos.x + e.worldPos.y, [&]() {
                e.draw(window);
            }});
        }

        // Добавляем игрока
        renderQueue.push_back({ player.pos.x + player.pos.y, [&]() {
            sf::CircleShape pShape(12);
            pShape.setFillColor(sf::Color::Cyan);
            pShape.setOrigin(12, 12);
            pShape.setPosition(IsoMath::worldToScreen(player.pos.x, player.pos.y));
            window.draw(pShape);
        }});

        // Сортируем очередь отрисовки по глубине (depth = x + y)
        std::sort(renderQueue.begin(), renderQueue.end(), [](const RenderObject& a, const RenderObject& b) {
            return a.depth < b.depth;
        });

        // Отрисовываем всё по порядку
        for (auto& obj : renderQueue) {
            obj.drawFunc();
        }

        window.display();
    }
    return 0;
}