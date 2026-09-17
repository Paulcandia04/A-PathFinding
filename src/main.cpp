#include <iostream>
#include <SFML/Graphics.hpp>
#include "Grid.hpp"
#include "AStar.hpp"
#include "Player.hpp"
#include "GameState.hpp"
#include "Renderer.hpp"
#include "InputHandler.hpp"
#include "Game.hpp"

int main() {
    Game game;
    if (!game.Initialize()) {
        std::cerr << "Error inicializando el juego" << std::endl;
        return -1;
    }
    game.Run();
    return 0;
}