#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include <SFML/Graphics.hpp>
#include "Grid.hpp"
#include "Player.hpp"
#include "GameState.hpp"
#include "AStar.hpp"

class InputHandler {
public:
    static sf::Vector2f ScreenToWorld(sf::Vector2i screenPos, float scale, sf::Vector2f offset) {
        return sf::Vector2f((screenPos.x - offset.x) / scale, (screenPos.y - offset.y) / scale);
    }

    static bool HandleEvent(const sf::Event& event, GameState& gameState, Grid& grid,
                             Player& player, float scale, sf::Vector2f offset) {

        if (gameState.GetState() == AppState::MENU) {
            if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Enter) gameState.SetState(AppState::PLAYING);
                else if (keyPressed->code == sf::Keyboard::Key::Escape) return false;
            }
            return true;
        }

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) gameState.SetState(AppState::MENU);
            else if (keyPressed->code == sf::Keyboard::Key::E) gameState.ToggleEditMode();
            else if (keyPressed->code == sf::Keyboard::Key::S && gameState.IsEditMode())
                grid.SaveToFile("assets/grid.txt");
        }

        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            sf::Vector2f world = ScreenToWorld(mousePressed->position, scale, offset);
            sf::Vector2i cell = grid.WorldToCell(world.x, world.y);

            if (gameState.IsEditMode()) {
                if (mousePressed->button == sf::Mouse::Button::Left) grid.SetCell(cell.x, cell.y, true);
                else if (mousePressed->button == sf::Mouse::Button::Right) grid.SetCell(cell.x, cell.y, false);
                        } else if (mousePressed->button == sf::Mouse::Button::Left) {
                sf::Vector2i target = grid.FindNearestWalkable(cell.x, cell.y);
                sf::Vector2i startCell = grid.WorldToCell(player.GetPosition().x, player.GetPosition().y);
                startCell = grid.FindNearestWalkable(startCell.x, startCell.y);

                std::vector<sf::Vector2f> path = AStar::FindPath(grid, startCell, target);
                if (!path.empty()) player.SetPreviewPath(path); // solo se dibuja, tú caminas con las flechas
            }
        }

        return true;
    }
};

#endif