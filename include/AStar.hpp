#ifndef ASTAR_HPP
#define ASTAR_HPP

#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "Grid.hpp"

class AStar {
private:
    struct NodeRecord {
        int col, row;
        float g, h, f;
        int parentCol, parentRow;
    };

    struct CompareNode {
        bool operator()(const NodeRecord* a, const NodeRecord* b) const {
            return a->f > b->f;
        }
    };

    static long Key(int col, int row, int cols) { return (long)row * cols + col; }

    static float Heuristic(int c1, int r1, int c2, int r2) {
        float dx = (float)(c1 - c2), dy = (float)(r1 - r2);
        return std::sqrt(dx * dx + dy * dy);
    }

public:
    static std::vector<sf::Vector2f> FindPath(const Grid& grid, sf::Vector2i start, sf::Vector2i goal) {
        std::vector<sf::Vector2f> result;
        if (!grid.IsInBounds(start.x, start.y) || !grid.IsInBounds(goal.x, goal.y)) return result;
        if (!grid.IsWalkable(goal.x, goal.y)) return result;

        int cols = grid.GetCols();
        std::priority_queue<NodeRecord*, std::vector<NodeRecord*>, CompareNode> openQueue;
        std::vector<NodeRecord*> allocated;
        std::unordered_map<long, bool> closed;
        std::unordered_map<long, float> bestG;
        std::unordered_map<long, sf::Vector2i> cameFrom;

        auto makeNode = [&](int c, int r, float g, float h) {
            NodeRecord* n = new NodeRecord{ c, r, g, h, g + h, 0, 0 };
            allocated.push_back(n);
            return n;
        };

        openQueue.push(makeNode(start.x, start.y, 0.0f, Heuristic(start.x, start.y, goal.x, goal.y)));
        bestG[Key(start.x, start.y, cols)] = 0.0f;

        const int dCol[8] = { 1,-1, 0, 0, 1, 1,-1,-1 };
        const int dRow[8] = { 0, 0, 1,-1, 1,-1, 1,-1 };
        bool found = false;

        while (!openQueue.empty()) {
            NodeRecord* current = openQueue.top();
            openQueue.pop();
            long ck = Key(current->col, current->row, cols);
            if (closed[ck]) continue;
            closed[ck] = true;

            if (current->col == goal.x && current->row == goal.y) { found = true; break; }

            for (int i = 0; i < 8; i++) {
                int nc = current->col + dCol[i], nr = current->row + dRow[i];
                if (!grid.IsWalkable(nc, nr)) continue;

                if (dCol[i] != 0 && dRow[i] != 0) { // evita cortar esquinas de edificios
                    if (!grid.IsWalkable(current->col + dCol[i], current->row) ||
                        !grid.IsWalkable(current->col, current->row + dRow[i])) continue;
                }

                float stepCost = (dCol[i] != 0 && dRow[i] != 0) ? 1.4142f : 1.0f;
                float tentativeG = current->g + stepCost;
                long nk = Key(nc, nr, cols);
                if (closed.count(nk) && closed[nk]) continue;

                if (!bestG.count(nk) || tentativeG < bestG[nk]) {
                    bestG[nk] = tentativeG;
                    openQueue.push(makeNode(nc, nr, tentativeG, Heuristic(nc, nr, goal.x, goal.y)));
                    cameFrom[nk] = sf::Vector2i(current->col, current->row);
                }
            }
        }

        if (found) {
            std::vector<sf::Vector2i> cellPath;
            sf::Vector2i cur(goal.x, goal.y);
            cellPath.push_back(cur);
            while (!(cur.x == start.x && cur.y == start.y)) {
                long k = Key(cur.x, cur.y, cols);
                if (!cameFrom.count(k)) break;
                cur = cameFrom[k];
                cellPath.push_back(cur);
            }
            std::reverse(cellPath.begin(), cellPath.end());
            for (auto& cell : cellPath) result.push_back(grid.CellToWorldCenter(cell.x, cell.y));
        }

        for (auto* n : allocated) delete n;
        return result;
    }
};

#endif