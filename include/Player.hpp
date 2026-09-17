#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <array>
#include <cmath>
#include <fstream>
#include <optional>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Grid.hpp"

enum class Direction { Down = 0, Up = 1, Left = 2, Right = 3 };

class Player {
private:
    
    std::array<std::array<std::optional<sf::Texture>, 2>, 4> m_walkTex;
    sf::Texture m_fallbackTexture;
    bool m_hasFallback = false;

    sf::Texture m_movingTexture;
    bool m_hasMovingTexture = false;

    std::optional<sf::Sprite> m_sprite;
    sf::Vector2f m_position;
    float m_speed;
    float m_baseScale;

    Direction m_direction = Direction::Down;
    bool m_isMoving = false;
    int m_animFrame = 0;
    float m_animTimer = 0.f;
    const float m_animInterval = 0.15f;

    std::vector<sf::Vector2f> m_previewPath;

    static bool FileExists(const std::string& path) {
        std::ifstream f(path);
        return f.good();
    }

public:
    Player() : m_position(0.f, 0.f), m_speed(160.0f), m_baseScale(0.04f) {}

    bool LoadTexture(const std::string& file) {
        if (!m_fallbackTexture.loadFromFile(file)) return false;
        m_hasFallback = true;
        m_sprite.emplace(m_fallbackTexture);
        sf::FloatRect b = m_sprite->getLocalBounds();
        m_sprite->setOrigin(sf::Vector2f(b.size.x / 2.0f, b.size.y / 2.0f));
        return true;
    }

    void TryLoadWalkFrames() {
        struct Entry { Direction dir; const char* name; };
        Entry dirs[4] = {
            { Direction::Down,  "down"  },
            { Direction::Up,    "up"    },
            { Direction::Left,  "left"  },
            { Direction::Right, "right" }
        };
        for (auto& e : dirs) {
            for (int frame = 0; frame < 2; frame++) {
                std::string path = "assets/player_" + std::string(e.name) + "_" + std::to_string(frame + 1) + ".png";
                if (FileExists(path)) {
                    sf::Texture tex;
                    if (tex.loadFromFile(path)) {
                        m_walkTex[(int)e.dir][frame] = tex;
                    }
                }
            }
        }

        if (FileExists("assets/Player2.png")) {
            if (m_movingTexture.loadFromFile("assets/Player2.png")) {
                m_hasMovingTexture = true;
            }
        }
    }

    void SetPosition(sf::Vector2f pos) { m_position = pos; }
    sf::Vector2f GetPosition() const { return m_position; }
    void SetSpeed(float s) { m_speed = s; }
    void SetBaseScale(float s) { m_baseScale = s; }

    void SetPreviewPath(const std::vector<sf::Vector2f>& path) { m_previewPath = path; }
    const std::vector<sf::Vector2f>& GetPath() const { return m_previewPath; }

    void CheckPathArrival() {
        if (m_previewPath.empty()) return;
        sf::Vector2f target = m_previewPath.back();
        float dist = std::hypot(target.x - m_position.x, target.y - m_position.y);
        if (dist < 12.0f) {
            m_previewPath.clear();
        }
    }

    void HandleKeyboardMovement(float dt, const Grid& grid) {
        bool up    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
        bool down  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
        bool left  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);

        sf::Vector2f dir(0.f, 0.f);
        if (up)    { dir.y -= 1.f; m_direction = Direction::Up; }
        if (down)  { dir.y += 1.f; m_direction = Direction::Down; }
        if (left)  { dir.x -= 1.f; m_direction = Direction::Left; }
        if (right) { dir.x += 1.f; m_direction = Direction::Right; }

        m_isMoving = (dir.x != 0.f || dir.y != 0.f);

        if (m_isMoving) {
            float len = std::hypot(dir.x, dir.y);
            dir.x /= len; dir.y /= len;

            sf::Vector2f moved = m_position + dir * m_speed * dt;

            sf::Vector2i cellX = grid.WorldToCell(moved.x, m_position.y);
            sf::Vector2i cellY = grid.WorldToCell(m_position.x, moved.y);

            if (grid.IsWalkable(cellX.x, cellX.y)) m_position.x = moved.x;
            if (grid.IsWalkable(cellY.x, cellY.y)) m_position.y = moved.y;

            m_animTimer += dt;
            if (m_animTimer >= m_animInterval) {
                m_animTimer = 0.f;
                m_animFrame = 1 - m_animFrame;
            }
        } else {
            m_animTimer = 0.f;
            m_animFrame = 0;
        }

        CheckPathArrival();
    }

    void Draw(sf::RenderWindow& window, float scale, sf::Vector2f offset) {
        if (!m_sprite.has_value()) return;

        int di = (int)m_direction;
        int frame = m_isMoving ? m_animFrame : 0;
        float flipX = 1.0f;

        if (m_walkTex[di][frame].has_value()) {
            m_sprite->setTexture(*m_walkTex[di][frame], true);
        } else if (m_isMoving && m_hasMovingTexture) {
            m_sprite->setTexture(m_movingTexture, true);
            if (m_direction == Direction::Left) flipX = -1.0f;
        } else if (m_hasFallback) {
            m_sprite->setTexture(m_fallbackTexture, true);
            if (m_direction == Direction::Left) flipX = -1.0f;
        }

        sf::FloatRect b = m_sprite->getLocalBounds();
        m_sprite->setOrigin(sf::Vector2f(b.size.x / 2.0f, b.size.y / 2.0f));

    
        sf::Vector2f targetSizeRatio(1.0f, 1.0f);
        if (m_hasFallback && b.size.x > 0.f && b.size.y > 0.f) {
            sf::Vector2u baseSize = m_fallbackTexture.getSize();
            targetSizeRatio.x = static_cast<float>(baseSize.x) / b.size.x;
            targetSizeRatio.y = static_cast<float>(baseSize.y) / b.size.y;
        }

        m_sprite->setScale(sf::Vector2f(
            m_baseScale * scale * flipX * targetSizeRatio.x, 
            m_baseScale * scale * targetSizeRatio.y
        ));
        
        m_sprite->setPosition(sf::Vector2f(offset.x + m_position.x * scale, offset.y + m_position.y * scale));
        window.draw(*m_sprite);
    }
};

#endif