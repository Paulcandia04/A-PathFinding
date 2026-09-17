#ifndef GRID_HPP
#define GRID_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>

class Grid {
private:
    int m_cols;
    int m_rows;
    int m_cellSize; 
    std::vector<std::vector<bool>> m_walkable; 

    bool ClassifyPixelAsPath(const sf::Color& c) const {
        if (c.a < 200) return false; 

        
        if (c.g > c.r + 10 && c.g > c.b + 25) return false;

        
        if (c.b > c.r + 15 && c.b > c.g + 5) return false;

        
        bool road = (c.r > 140 && c.r < 240) &&
                    (c.g > 100 && c.g < 210) &&
                    (c.b > 60  && c.b < 180) &&
                    (c.r >= c.b);
        return road;
    }

public:
    Grid() : m_cols(0), m_rows(0), m_cellSize(20) {}

    void BuildFromImage(const sf::Image& cityImage, int cellSize) {
        m_cellSize = cellSize;
        sf::Vector2u size = cityImage.getSize();
        m_cols = size.x / cellSize;
        m_rows = size.y / cellSize;
        m_walkable.assign(m_rows, std::vector<bool>(m_cols, false));

        for (int row = 0; row < m_rows; row++) {
            for (int col = 0; col < m_cols; col++) {
                unsigned int px = col * cellSize + cellSize / 2;
                unsigned int py = row * cellSize + cellSize / 2;
                sf::Color c = cityImage.getPixel(sf::Vector2u(px, py));
                m_walkable[row][col] = ClassifyPixelAsPath(c);
            }
        }
    }

    bool SaveToFile(const std::string& path) const {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << m_cols << " " << m_rows << " " << m_cellSize << "\n";
        for (int row = 0; row < m_rows; row++) {
            for (int col = 0; col < m_cols; col++)
                file << (m_walkable[row][col] ? '1' : '0');
            file << "\n";
        }
        return true;
    }

    bool LoadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        file >> m_cols >> m_rows >> m_cellSize;
        file.ignore();
        m_walkable.assign(m_rows, std::vector<bool>(m_cols, false));
        std::string line;
        for (int row = 0; row < m_rows; row++) {
            std::getline(file, line);
            for (int col = 0; col < m_cols && col < (int)line.size(); col++)
                m_walkable[row][col] = (line[col] == '1');
        }
        return true;
    }

    void SetCell(int col, int row, bool value) {
        if (IsInBounds(col, row)) m_walkable[row][col] = value;
    }

    bool IsInBounds(int col, int row) const {
        return col >= 0 && col < m_cols && row >= 0 && row < m_rows;
    }

    bool IsWalkable(int col, int row) const {
        if (!IsInBounds(col, row)) return false;
        return m_walkable[row][col];
    }

    sf::Vector2i WorldToCell(float worldX, float worldY) const {
        return sf::Vector2i((int)(worldX / m_cellSize), (int)(worldY / m_cellSize));
    }

    sf::Vector2f CellToWorldCenter(int col, int row) const {
        return sf::Vector2f(col * m_cellSize + m_cellSize / 2.0f,
                             row * m_cellSize + m_cellSize / 2.0f);
    }

    
    sf::Vector2i FindNearestWalkable(int col, int row, int maxRadius = 15) const {
        if (IsWalkable(col, row)) return sf::Vector2i(col, row);
        for (int radius = 1; radius <= maxRadius; radius++) {
            for (int dx = -radius; dx <= radius; dx++) {
                for (int dy = -radius; dy <= radius; dy++) {
                    if (std::max(std::abs(dx), std::abs(dy)) != radius) continue;
                    if (IsWalkable(col + dx, row + dy)) return sf::Vector2i(col + dx, row + dy);
                }
            }
        }
        return sf::Vector2i(col, row);
    }

    int GetCols() const { return m_cols; }
    int GetRows() const { return m_rows; }
    int GetCellSize() const { return m_cellSize; }
};

#endif