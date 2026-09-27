#pragma once
#include "math/Vec3.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <cmath>

class SpatialHashGrid {
public:
    explicit SpatialHashGrid(float cellSize) : m_cellSize(cellSize) {}

    void clear();
    void insert(int particleIndex, const Vec3& position);
    std::vector<int> getNeighbours(const Vec3& position) const;

private:
    float m_cellSize;

    struct CellKey {
        int x, y, z;
        bool operator==(const CellKey& o) const {
            return x == o.x && y == o.y && z == o.z;
        }
    };

    struct CellKeyHash {
        size_t operator()(const CellKey& k) const {
            return std::hash<int>()(k.x * 73856093) ^
                   std::hash<int>()(k.y * 19349663) ^
                   std::hash<int>()(k.z * 83492791);
        }
    };

    std::unordered_map<CellKey, std::vector<int>, CellKeyHash> m_cells;

    CellKey getCell(const Vec3& pos) const {
        return {
            (int)std::floor(pos.x / m_cellSize),
            (int)std::floor(pos.y / m_cellSize),
            (int)std::floor(pos.z / m_cellSize)
        };
    }
};
