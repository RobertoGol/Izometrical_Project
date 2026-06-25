#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include "IsoMath.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT), "Izometrical Project - C++");
    window.setFramerateLimit(60);

    Player player;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mPos = IsoMath::screenToWorld(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
                player.shoot(mPos);
            }
        }

        player.update(window);

        window.clear(sf::Color(30, 30, 30));

        // 1. Рисуем сетку пола
        for (int x = 0; x < 15; x++) {
            for (int y = 0; y < 15; y++) {
                sf::ConvexShape tile(4);
                tile.setPoint(0, IsoMath::worldToScreen(x, y));
                tile.setPoint(1, IsoMath::worldToScreen(x+1, y));
                tile.setPoint(2, IsoMath::worldToScreen(x+1, y+1));
                tile.setPoint(3, IsoMath::worldToScreen(x, y+1));
                tile.setFillColor(sf::Color(50, 50, 50));
                tile.setOutlineThickness(1);
                tile.setOutlineColor(sf::Color(70, 70, 70));
                window.draw(tile);
            }
        }

        // 2. Рисуем игрока
        sf::CircleShape pShape(10);
        pShape.setFillColor(sf::Color::Cyan);
        sf::Vector2f pScreen = IsoMath::worldToScreen(player.pos.x, player.pos.y);
        pShape.setOrigin(10, 10);
        pShape.setPosition(pScreen);
        window.draw(pShape);

        // 3. Рисуем пули
        for (auto& b : player.bullets) {
            sf::CircleShape bShape(3);
            bShape.setFillColor(sf::Color::Yellow);
            bShape.setPosition(IsoMath::worldToScreen(b.worldPos.x, b.worldPos.y));
            window.draw(bShape);
        }

        window.display();
    }
    return 0;
}