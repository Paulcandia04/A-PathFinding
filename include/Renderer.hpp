#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <optional>
#include <vector>
#include <iostream>
#include "Grid.hpp"
#include "Player.hpp"
#include "GameState.hpp"

class Renderer {
private:
    sf::RenderWindow* m_window;
    sf::Font m_font;
    sf::Font m_fontBold;
    sf::Texture m_cityTexture;
    std::optional<sf::Sprite> m_citySprite;
    float m_scale;
    sf::Vector2f m_offset;

public:
    Renderer(sf::RenderWindow* win) : m_window(win), m_scale(1.0f), m_offset(0.f, 0.f) {}

    bool LoadAssets() {
        bool ok = true;
        if (!m_font.openFromFile("assets/OpenSans-Regular.ttf")) { std::cerr << "Falta OpenSans-Regular.ttf\n"; ok = false; }
        if (!m_fontBold.openFromFile("assets/OpenSans-Bold.ttf")) { std::cerr << "Falta OpenSans-Bold.ttf\n"; ok = false; }
        if (!m_cityTexture.loadFromFile("assets/Ciudad.png")) { std::cerr << "Falta Ciudad.png\n"; ok = false; }
        else { m_citySprite.emplace(m_cityTexture); }
        return ok;
    }

    void RecalculateLayout() {
        if (!m_citySprite.has_value()) return;
        sf::Vector2u winSize = m_window->getSize();
        sf::Vector2u imgSize = m_cityTexture.getSize();
        float scaleX = (float)winSize.x / imgSize.x;
        float scaleY = (float)winSize.y / imgSize.y;
        m_scale = std::min(scaleX, scaleY);

        float drawnW = imgSize.x * m_scale, drawnH = imgSize.y * m_scale;
        m_offset.x = (winSize.x - drawnW) / 2.0f;
        m_offset.y = (winSize.y - drawnH) / 2.0f;

        m_citySprite->setScale(sf::Vector2f(m_scale, m_scale));
        m_citySprite->setPosition(m_offset);
    }

    float GetScale() const { return m_scale; }
    sf::Vector2f GetOffset() const { return m_offset; }

    void Clear() { m_window->clear(sf::Color(20, 20, 20)); }
    void Display() { m_window->display(); }
    void DrawCity() { if (m_citySprite.has_value()) m_window->draw(*m_citySprite); }

    void DrawGridOverlay(const Grid& grid) {
        sf::VertexArray triangles(sf::PrimitiveType::Triangles);
        int cell = grid.GetCellSize();
        for (int row = 0; row < grid.GetRows(); row++) {
            for (int col = 0; col < grid.GetCols(); col++) {
                sf::Color c = grid.IsWalkable(col, row) ? sf::Color(0, 255, 0, 60) : sf::Color(255, 0, 0, 60);
                float x = m_offset.x + col * cell * m_scale;
                float y = m_offset.y + row * cell * m_scale;
                float s = cell * m_scale;

                sf::Vertex v0, v1, v2, v3;
                v0.position = sf::Vector2f(x, y);         v0.color = c;
                v1.position = sf::Vector2f(x + s, y);     v1.color = c;
                v2.position = sf::Vector2f(x + s, y + s); v2.color = c;
                v3.position = sf::Vector2f(x, y + s);     v3.color = c;

                triangles.append(v0);
                triangles.append(v1);
                triangles.append(v2);

                triangles.append(v0);
                triangles.append(v2);
                triangles.append(v3);
            }
        }
        m_window->draw(triangles);
    }

        void DrawPath(const std::vector<sf::Vector2f>& path) {
        if (path.size() < 2) return;

        float thickness = 10.0f;
        sf::VertexArray triangles(sf::PrimitiveType::Triangles);
        sf::Color pathColor(255, 200, 0, 235);

        for (size_t i = 0; i + 1 < path.size(); i++) {
            sf::Vector2f p1(m_offset.x + path[i].x * m_scale, m_offset.y + path[i].y * m_scale);
            sf::Vector2f p2(m_offset.x + path[i + 1].x * m_scale, m_offset.y + path[i + 1].y * m_scale);

            sf::Vector2f dir = p2 - p1;
            float len = std::hypot(dir.x, dir.y);
            if (len < 0.0001f) continue;

            sf::Vector2f normal(-dir.y / len, dir.x / len);
            sf::Vector2f half = normal * (thickness / 2.0f);

            sf::Vertex v1, v2, v3, v4;
            v1.position = p1 + half; v1.color = pathColor;
            v2.position = p1 - half; v2.color = pathColor;
            v3.position = p2 + half; v3.color = pathColor;
            v4.position = p2 - half; v4.color = pathColor;

            triangles.append(v1); triangles.append(v2); triangles.append(v3);
            triangles.append(v2); triangles.append(v4); triangles.append(v3);
        }
        m_window->draw(triangles);

        sf::CircleShape dot(9.0f);
        dot.setFillColor(sf::Color(220, 30, 30));
        dot.setOutlineColor(sf::Color::White);
        dot.setOutlineThickness(2.0f);
        dot.setOrigin(sf::Vector2f(9.0f, 9.0f));
        dot.setPosition(sf::Vector2f(m_offset.x + path.back().x * m_scale, m_offset.y + path.back().y * m_scale));
        m_window->draw(dot);
    }

    void DrawPlayer(Player& player) { player.Draw(*m_window, m_scale, m_offset); }

    void DrawHUD(const GameState& gameState) {
        sf::Text info(m_font, gameState.IsEditMode()
            ? "MODO EDICION: Click izq=caminable | Click der=bloqueado | S=guardar | E=salir"
            : "Click en el mapa para trazar la ruta | E=editar calles | ESC=menu",
            18);
        info.setPosition(sf::Vector2f(10, 10));
        info.setFillColor(sf::Color::White);
        info.setOutlineColor(sf::Color::Black);
        info.setOutlineThickness(2.0f);
        m_window->draw(info);
    }

    void DrawNameTag() {
        sf::Text name(m_fontBold, "Paul Candia", 22);
        sf::FloatRect b = name.getLocalBounds();
        name.setPosition(sf::Vector2f((float)m_window->getSize().x - b.size.x - 25, 15));
        name.setFillColor(sf::Color::White);
        m_window->draw(name);

        sf::Text reg(m_font, "No. Registro: 24110244", 16);
        sf::FloatRect rb = reg.getLocalBounds();
        reg.setPosition(sf::Vector2f((float)m_window->getSize().x - rb.size.x - 25, 42));
        reg.setFillColor(sf::Color(220, 220, 220));
        m_window->draw(reg);
    }

    void DrawMainMenu() {
        m_window->clear(sf::Color(15, 15, 25));
        sf::Vector2u win = m_window->getSize();

        sf::Text title(m_fontBold, "A* Pathfinding - Ciudad", 60);
        sf::FloatRect tb = title.getLocalBounds();
        title.setPosition(sf::Vector2f(((float)win.x - tb.size.x) / 2.0f, win.y * 0.25f));
        title.setFillColor(sf::Color(255, 200, 60));
        m_window->draw(title);

        sf::Text playOpt(m_fontBold, "JUGAR  (ENTER)", 40);
        sf::FloatRect pb = playOpt.getLocalBounds();
        playOpt.setPosition(sf::Vector2f(((float)win.x - pb.size.x) / 2.0f, win.y * 0.45f));
        playOpt.setFillColor(sf::Color::White);
        m_window->draw(playOpt);

        sf::Text exitOpt(m_fontBold, "SALIR  (ESC)", 40);
        sf::FloatRect eb = exitOpt.getLocalBounds();
        exitOpt.setPosition(sf::Vector2f(((float)win.x - eb.size.x) / 2.0f, win.y * 0.45f + 70));
        exitOpt.setFillColor(sf::Color(200, 200, 200));
        m_window->draw(exitOpt);

        DrawNameTag();
        m_window->display();
    }

    void RenderPlayingFrame(Grid& grid, Player& player, GameState& gameState) {
        Clear();
        DrawCity();
        if (gameState.IsEditMode()) DrawGridOverlay(grid);
        DrawPath(player.GetPath());
        DrawPlayer(player);
        DrawHUD(gameState);
        DrawNameTag();
        Display();
    }
};

#endif