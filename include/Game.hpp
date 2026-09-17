#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <optional>
#include "Grid.hpp"
#include "AStar.hpp"
#include "Player.hpp"
#include "GameState.hpp"
#include "Renderer.hpp"
#include "InputHandler.hpp"

class Game {
private:
    sf::RenderWindow window;
    GameState gameState;
    Renderer renderer;
    Grid grid;
    Player player;
    sf::Clock deltaClock;

    void InitGrid() {
        const std::string gridFile = "assets/grid.txt";
        std::ifstream test(gridFile);
        if (test.good()) {
            test.close();
            grid.LoadFromFile(gridFile);
            std::cout << "Cuadricula cargada desde " << gridFile << std::endl;
        } else {
            sf::Image cityImage;
            if (!cityImage.loadFromFile("assets/Ciudad.png")) {
                std::cerr << "No se pudo cargar Ciudad.png\n";
                return;
            }
            grid.BuildFromImage(cityImage, 20);
            grid.SaveToFile(gridFile);
            std::cout << "Cuadricula generada automaticamente. Presiona 'E' en el juego\n"
                       << "para revisar/corregir las calles a mano y 'S' para guardar.\n";
        }
    }

public:
    Game() : window(sf::VideoMode(sf::Vector2u(1400, 900)), "A* Pathfinding - Ciudad"), renderer(&window) {
        window.setFramerateLimit(60);
    }

    bool Initialize() {
        if (!renderer.LoadAssets()) return false;
        if (!player.LoadTexture("assets/player.png")) return false;
        player.TryLoadWalkFrames();

        InitGrid();
        renderer.RecalculateLayout();

        sf::Vector2i centerCell(grid.GetCols() / 2, grid.GetRows() / 2);
        centerCell = grid.FindNearestWalkable(centerCell.x, centerCell.y);
        player.SetPosition(grid.CellToWorldCenter(centerCell.x, centerCell.y));

        return true;
    }

    void Run() {
        while (window.isOpen()) {
            float deltaTime = deltaClock.restart().asSeconds();

            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) window.close();

                if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                    window.setView(sf::View(sf::FloatRect(
                        sf::Vector2f(0.f, 0.f),
                        sf::Vector2f((float)resized->size.x, (float)resized->size.y))));
                    renderer.RecalculateLayout();
                }

                if (!InputHandler::HandleEvent(*event, gameState, grid, player, renderer.GetScale(), renderer.GetOffset()))
                    window.close();
            }

                if (gameState.GetState() == AppState::PLAYING) {
                player.HandleKeyboardMovement(deltaTime, grid);
                renderer.RenderPlayingFrame(grid, player, gameState);
            } else {
                renderer.DrawMainMenu();
            }
        }
    }
};

#endif